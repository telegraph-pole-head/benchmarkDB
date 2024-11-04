#ifndef PROJECT_DB_BPTREE_TPP
#define PROJECT_DB_BPTREE_TPP

#include "BpTree.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

// Find the leaf node for index
template <typename IndexType, typename DataType>
typename BpTree<IndexType, DataType>::Node *
BpTree<IndexType, DataType>::findLeafNode(const IndexType &index) {
  Node *current = root.get();
  while (!current->isLeaf) {
    auto &children = current->ptrChildrenOrData.children;
    auto it = std::lower_bound(current->indexes.begin(), current->indexes.end(),
                               index); // binary search to find the first
                                       // element greater than index
    auto idxChild = std::distance(current->indexes.begin(), it);
    // set the current node to the last element less than index
    current = children[(size_t)idxChild].get();
  }
  return current;
}

// Find the parent of a node
template <typename IndexType, typename DataType>
typename BpTree<IndexType, DataType>::Node *
BpTree<IndexType, DataType>::findParent(Node *child) {
  if (child == root.get())
    return nullptr;
  Node *current = root.get();
  Node *parent = nullptr;
  while (current != child) {
    parent = current;
    auto it = std::lower_bound(current->indexes.begin(), current->indexes.end(),
                               child->indexes.back());
    auto idx = std::distance(current->indexes.begin(), it);
    current = current->ptrChildrenOrData.children[(size_t)idx].get();
  }
  return parent;
}

// Get the leftmost leaf node
template <typename IndexType, typename DataType>
typename BpTree<IndexType, DataType>::Node *
BpTree<IndexType, DataType>::getLeftmostLeaf() {
  Node *current = root.get();
  while (!current->isLeaf) {
    current = current->ptrChildrenOrData.children[0].get();
  }
  return current;
}

// Remove the index and data from the node
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::removeFromNode(Node *node, size_t pos) {
  node->indexes.erase(node->indexes.begin() + (long)pos);
  if (node->isLeaf) {
    node->ptrChildrenOrData.data.erase(node->ptrChildrenOrData.data.begin() +
                                       (long)pos);
  } else {
    node->ptrChildrenOrData.children.erase(
        node->ptrChildrenOrData.children.begin() + (long)pos + 1);
  }
}

// Split the leaf node
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::splitLeafNode(
    Node *leaf, const IndexType &index, std::unique_ptr<DataType> data) {
  NodePtr newLeaf = std::make_unique<Node>(true); // create a new leaf node
  auto splitPoint = static_cast<long>(leaf->indexes.size() / 2);
  newLeaf->indexes.assign(leaf->indexes.begin() + splitPoint,
                          leaf->indexes.end());
  newLeaf->ptrChildrenOrData.data.assign(
      std::make_move_iterator(leaf->ptrChildrenOrData.data.begin() +
                              splitPoint),
      std::make_move_iterator(leaf->ptrChildrenOrData.data.end()));

  // resize the original leaf
  leaf->indexes.resize((size_t)splitPoint);
  leaf->ptrChildrenOrData.data.resize((size_t)splitPoint);

  // update the next pointer
  newLeaf->next = std::move(leaf->next);
  leaf->next = std::move(newLeaf);

  // Promote the smallest index of the new leaf to the parent
  promoteToParent(leaf, newLeaf->indexes.front(), std::move(newLeaf));
}

// Promote the child to parent
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::promoteToParent(Node *left,
                                                  const IndexType &index,
                                                  NodePtr right) {
  if (left == root.get()) {
    NodePtr newRoot = std::make_unique<Node>(false);
    // set the indexes and children
    newRoot->indexes.emplace_back(index);
    newRoot->ptrChildrenOrData.children.emplace_back(std::move(left));
    newRoot->ptrChildrenOrData.children.emplace_back(std::move(right));
    // when internal node is generated, it has 1 index and 2 children
    // so num_children = num_indexes + 1
    root = std::move(newRoot);
    return;
  }

  Node *parent = findParent(left);

  // insert the index and the right child
  auto it =
      std::lower_bound(parent->indexes.begin(), parent->indexes.end(), index);
  auto idx = std::distance(parent->indexes.begin(), it);
  parent->indexes.insert(it, index);
  auto idxChild = parent->ptrChildrenOrData.children.begin() + idx + 1;
  parent->ptrChildrenOrData.children.insert(idxChild, std::move(right));

  // check if the parent is overflowing
  if (parent->indexes.size() >= maxIntChildren) {
    splitInternalNode(parent);
  }
}

// Split the internal node
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::splitInternalNode(Node *internal) {
  // create a new internal node
  NodePtr newInternal = std::make_unique<Node>(false);
  // calculate the split point
  auto splitPoint =
      static_cast<typename std::vector<IndexType>::difference_type>(
          internal->indexes.size() / 2 + internal->indexes.size() % 2 - 1);

  // move half of the indexes and children to the new internal node
  newInternal->indexes.assign(internal->indexes.begin() + splitPoint + 1,
                              internal->indexes.end());
  newInternal->ptrChildrenOrData.children.assign(
      std::make_move_iterator(internal->ptrChildrenOrData.children.begin() +
                              splitPoint + 1),
      std::make_move_iterator(internal->ptrChildrenOrData.children.end()));

  // get the middle index to be promoted
  IndexType promotedIndex = internal->indexes[(size_t)splitPoint];

  // resize the original internal
  internal->indexes.resize((size_t)splitPoint);
  internal->ptrChildrenOrData.children.resize((size_t)splitPoint + 1);

  // Promote the child to parent
  promoteToParent(internal, promotedIndex, std::move(newInternal));
}

// Rebalance the tree
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::rebalance(Node *node) {
  // check if the leaf node is underflowing
  size_t minSize = maxLeafIdxes / 2;
  if (node->indexes.size() >= minSize)
    return;
  // handle underflow
  // handle level decrease triggered by underflow
  if (node == root.get()) {
    if (!node->isLeaf && node->indexes.empty()) {
      root = std::move(node->ptrChildrenOrData.children[0]);
    }
    return;
  }
  // get the parent, left and right siblings
  Node *parent = findParent(node);
  auto it = std::lower_bound(parent->indexes.begin(), parent->indexes.end(),
                             node->indexes.back());
  size_t idx = (size_t)std::distance(parent->indexes.begin(), it);
  Node *leftSibling =
      (idx > 0) ? parent->ptrChildrenOrData.children[idx - 1].get() : nullptr;
  Node *rightSibling = (idx < parent->indexes.size())
                           ? parent->ptrChildrenOrData.children[idx + 1].get()
                           : nullptr;
  if (leftSibling && leftSibling->indexes.size() > minSize) {
    borrowFromLeft(node, leftSibling, parent, idx - 1);
  } else if (rightSibling && rightSibling->indexes.size() > minSize) {
    borrowFromRight(node, rightSibling, parent, idx);
  } else if (leftSibling) {
    mergeNodes(leftSibling, node, parent, idx - 1);
    rebalance(parent);
  } else {
    mergeNodes(node, rightSibling, parent, idx);
    rebalance(parent);
  }
}

// Borrow a node from the left sibling
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::borrowFromLeft(Node *node, Node *leftSibling,
                                                 Node *parent, size_t idx) {
  if (node->isLeaf) {
    node->indexes.insert(node->indexes.begin(), leftSibling->indexes.back());
    node->ptrChildrenOrData.data.insert(
        node->ptrChildrenOrData.data.begin(),
        std::move(leftSibling->ptrChildrenOrData.data.back()));
    leftSibling->indexes.pop_back();
    leftSibling->ptrChildrenOrData.data.pop_back();
    // update the parent index
    parent->indexes[idx] = node->indexes.front();
  } else {
    node->indexes.insert(node->indexes.begin(), parent->indexes[idx]);
    node->ptrChildrenOrData.children.insert(
        node->ptrChildrenOrData.children.begin(),
        std::move(leftSibling->ptrChildrenOrData.children.back()));
    leftSibling->indexes.pop_back();
    leftSibling->ptrChildrenOrData.children.pop_back();
    // update the parent index
    parent->indexes[idx] = leftSibling->indexes.back();
  }
}

// Borrow a node from the right sibling
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::borrowFromRight(Node *node,
                                                  Node *rightSibling,
                                                  Node *parent, size_t idx) {
  if (node->isLeaf) {
    node->indexes.emplace_back(rightSibling->indexes.front());
    node->ptrChildrenOrData.data.emplace_back(
        std::move(rightSibling->ptrChildrenOrData.data.front()));
    rightSibling->indexes.erase(rightSibling->indexes.begin());
    rightSibling->ptrChildrenOrData.data.erase(
        rightSibling->ptrChildrenOrData.data.begin());
    parent->indexes[idx] = rightSibling->indexes.front();
  } else {
    node->indexes.emplace_back(parent->indexes[idx]);
    node->ptrChildrenOrData.children.emplace_back(
        std::move(rightSibling->ptrChildrenOrData.children.front()));
    rightSibling->indexes.erase(rightSibling->indexes.begin());
    rightSibling->ptrChildrenOrData.children.erase(
        rightSibling->ptrChildrenOrData.children.begin());
    parent->indexes[idx] = rightSibling->indexes.front();
  }
}

// Merge the sibling nodes
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::mergeNodes(Node *left, Node *right,
                                             Node *parent, size_t idx) {
  if (left->isLeaf) {
    // merge the indexes and data
    left->indexes.insert(left->indexes.end(), right->indexes.begin(),
                         right->indexes.end());
    left->ptrChildrenOrData.data.insert(
        left->ptrChildrenOrData.data.end(),
        std::make_move_iterator(right->ptrChildrenOrData.data.begin()),
        std::make_move_iterator(right->ptrChildrenOrData.data.end()));
    // update the next pointer
    left->next = std::move(right->next);
  } else {
    // merge the indexes and children
    left->indexes.emplace_back(parent->indexes[idx]);
    left->indexes.insert(left->indexes.end(), right->indexes.begin(),
                         right->indexes.end());
    left->ptrChildrenOrData.children.insert(
        left->ptrChildrenOrData.children.end(),
        std::make_move_iterator(right->ptrChildrenOrData.children.begin()),
        std::make_move_iterator(right->ptrChildrenOrData.children.end()));
  }
  removeFromNode(parent, idx);
}

// Insert a index-data pair into the B+ tree
template <typename IndexType, typename DataType>
bool BpTree<IndexType, DataType>::insert(const IndexType &index,
                                         const DataType &data) {
  std::cout
      << "\n------------------------------------------------------------\n";
  std::cout << "Starting insert: index = " << index << ", data = " << data
            << std::endl;

  // find the leaf node containing the index
  Node *leaf = findLeafNode(index);

  // std::cout << "Inserting into leaf node\n";

  bool isOverflow = (leaf->indexes.size() >= maxLeafIdxes);

  auto it = std::lower_bound(leaf->indexes.begin(), leaf->indexes.end(), index);
  auto diff = std::distance(leaf->indexes.begin(), it);
  std::cout << "Index is at position " << diff << std::endl;
  std::cout << "Indexes in the leaf node: ";
  for (auto i : leaf->indexes) {
    std::cout << i << " ";
  }
  std::cout << std::endl;
  if (it != leaf->indexes.end() && *it == index) {
    return false; // duplicate index
  }

  if (isOverflow) {
    std::cout << "Leaf node is overflowing\n";
  } else {
    std::cout << "Leaf node is not overflowing\n";
  }

  auto idx = std::distance(leaf->indexes.begin(), it);
  // insert the index and data (even if overflow, since we'll split later)
  leaf->indexes.insert(it, index);
  // auto idx_data = leaf->ptrChildrenOrData.data.begin() + idx;
  std::cout << "Inserting data into the " << idx << "th position\n";
  // get the pointer to the data
  auto ptr = std::make_unique<DataType>(data);
  auto it_data = leaf->ptrChildrenOrData.data.begin() + idx;

  leaf->ptrChildrenOrData.data.insert(it_data, std::move(ptr));

  if (isOverflow)
    splitLeafNode(leaf, index, std::make_unique<DataType>(data));

  return true;
}

// Remove a node with index from the B+ tree
template <typename IndexType, typename DataType>
bool BpTree<IndexType, DataType>::remove(const IndexType &index) {
  if (!root)
    return false;
  // find the leaf node containing the index
  Node *leaf = findLeafNode(index);
  auto it = std::lower_bound(leaf->indexes.begin(), leaf->indexes.end(), index);
  if (it == leaf->indexes.end() || *it != index) {
    return false; // Index not found
  }
  // remove the index and data
  auto idx = std::distance(leaf->indexes.begin(), it);
  removeFromNode(leaf, (size_t)idx);
  if ((leaf == root.get()) && (leaf->indexes.empty())) {
    root = std::make_unique<Node>(true);
    return true;
  }
  // rebalance the tree after deletion
  rebalance(leaf);
  return true;
}

// Search for a specific index
template <typename IndexType, typename DataType>
std::unique_ptr<DataType>
BpTree<IndexType, DataType>::search(const IndexType &index) {
  // find the leaf node containing the index
  std::unique_ptr<Node> leaf = std::make_unique<Node>(findLeafNode(index));
  // find the index in the leaf node
  auto it = std::lower_bound(leaf->indexes.begin(), leaf->indexes.end(), index);
  if (it != leaf->indexes.end() && *it == index) {
    auto idx = std::distance(leaf->indexes.begin(), it);
    return std::make_unique<DataType>(
        *leaf->ptrChildrenOrData.data[(size_t)idx]);
  }
  return nullptr;
}

// Get the minimum index in the B+ tree
template <typename IndexType, typename DataType>
std::unique_ptr<IndexType> BpTree<IndexType, DataType>::getMin() {
  if (!root)
    return nullptr;
  Node *current = getLeftmostLeaf();
  return std::make_unique<IndexType>(current->indexes.front());
}

// Get the maximum index in the B+ tree
template <typename IndexType, typename DataType>
std::unique_ptr<IndexType> BpTree<IndexType, DataType>::getMax() {
  if (!root)
    return nullptr;
  Node *current = root.get();
  while (!current->isLeaf) {
    current = current->ptrChildrenOrData.children.back().get();
  }
  return std::make_unique<IndexType>(current->indexes.back());
}

// Range query in the B+ tree
template <typename IndexType, typename DataType>
std::vector<std::unique_ptr<DataType>> BpTree<IndexType, DataType>::rangeQuery(
    const std::optional<IndexType> &minIndex,
    const std::optional<IndexType> &maxIndex) {
  std::vector<std::unique_ptr<DataType>> result;
  // Start from the leaf node containing minIndex or the leftmost leaf
  Node *current = minIndex ? findLeafNode(*minIndex) : getLeftmostLeaf();
  while (current) {
    size_t i = 0;
    // If minIndex is specified, find the starting point
    if (minIndex) {
      auto it = std::lower_bound(current->indexes.begin(),
                                 current->indexes.end(), *minIndex);
      i = (size_t)std::distance(current->indexes.begin(), it);
    }
    // Iterate through the leaf node
    for (; i < current->indexes.size(); ++i) {
      if (maxIndex && current->indexes[i] > *maxIndex)
        return result;
      result.emplace_back(
          std::make_unique<DataType>(*current->ptrChildrenOrData.data[i]));
    }
    // Move to the next leaf node
    current = current->next.get();
  }
  return result;
}

// Count the number of indexes in the range
template <typename IndexType, typename DataType>
size_t BpTree<IndexType, DataType>::countRange(
    const std::optional<IndexType> &minIndex,
    const std::optional<IndexType> &maxIndex) {
  size_t count = 0;
  // Start from the leaf node containing minIndex or the leftmost leaf
  Node *current = minIndex ? findLeafNode(*minIndex) : getLeftmostLeaf();
  while (current) {
    size_t i = 0;
    // If minIndex is specified, find the starting point
    if (minIndex) {
      auto it = std::lower_bound(current->indexes.begin(),
                                 current->indexes.end(), *minIndex);
      i = (size_t)std::distance(current->indexes.begin(), it);
    }
    // Iterate through the leaf node
    for (; i < current->indexes.size(); ++i) {
      if (maxIndex && current->indexes[i] > *maxIndex)
        return count;
      ++count;
    }
    // Move to the next leaf node
    current = current->next.get();
  }
  return count;
}

// Utility function to print the B+ Tree
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::printTree() const {
  if (!root)
    return;
  std::vector<Node *> currentLevel;
  std::vector<Node *> nextLevel;
  currentLevel.emplace_back(root.get());
  while (!currentLevel.empty()) {
    for (auto node : currentLevel) {
      std::cout << "[";
      for (size_t i = 0; i < node->indexes.size(); ++i) {
        std::cout << node->indexes[i];
        if (i != node->indexes.size() - 1)
          std::cout << ", ";
      }
      std::cout << "]";
      if (!node->isLeaf) {
        for (size_t i = 0; i < node->ptrChildrenOrData.children.size(); ++i) {
          nextLevel.emplace_back(node->ptrChildrenOrData.children[i].get());
        }
      }
    }
    std::cout << std::endl;
    currentLevel = nextLevel;
    nextLevel.clear();
  }
}

#endif
