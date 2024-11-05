#ifndef PROJECT_DB_BPTREEIMPL_H
#define PROJECT_DB_BPTREEIMPL_H

#pragma once
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
typename BpTree<IndexType, DataType>::NodePtr
BpTree<IndexType, DataType>::findLeafNode(const IndexType &index) const {
  if (root->isLeaf)
    return root;
  Node *current = root.get(); // get the raw pointer for faster traversal
  while (true) {
    auto &children = current->getChildren();
    auto it = std::upper_bound(current->indexes.begin(), current->indexes.end(),
                               index); // binary search to find the first
                                       // element greater than index
    size_t idxChild = (size_t)std::distance(current->indexes.begin(), it);
    // set the current node to the last element less than index
    if (children[idxChild]->isLeaf) {
      return children[idxChild];
    } else {
      current = children[idxChild].get();
    }
  }
}

// Find the parent of a node
template <typename IndexType, typename DataType>
typename BpTree<IndexType, DataType>::NodePtr
BpTree<IndexType, DataType>::findParent(NodePtr &child) const {
  if (child.get() == root.get())
    return nullptr;

  NodePtr parent = root;
  NodePtr current = root;
  while (!current->isLeaf) {
    parent = current;
    auto it = std::upper_bound(current->indexes.begin(), current->indexes.end(),
                               child->indexes.front());
    size_t idxChild = (size_t)std::distance(current->indexes.begin(), it);
    auto &closestChild = current->getChildren()[idxChild];
    if (closestChild.get() == child.get())
      return parent;
    current = closestChild;
  }
  return parent; // return the supposed parent if child is not found
}

// Get the leftmost leaf node
template <typename IndexType, typename DataType>
typename BpTree<IndexType, DataType>::NodePtr
BpTree<IndexType, DataType>::getLeftmostLeaf() const {
  if (root->isLeaf)
    return root;
  Node *current = root.get();
  while (true) {
    current = current->getChildren().front().get();
    if (current->isLeaf)
      return current->getShared();
  }
}

// Remove the index and data from the node
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::removeFromNode(NodePtr node, size_t pos) {
  node->indexes.erase(node->indexes.begin() + (long)pos);
  if (node->isLeaf) {
    node->getData().erase(node->getData().begin() + (long)pos);
  } else {
    node->getChildren().erase(node->getChildren().begin() + (long)pos + 1);
  }
}

// Split the leaf node
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::splitLeafNode(
    NodePtr leaf, const IndexType &index, std::shared_ptr<DataType> data) {
  NodePtr newLeaf = Node::createLeaf(); // create a new leaf node
  auto splitPoint = static_cast<long>(leaf->indexes.size() / 2);
  newLeaf->indexes.assign(leaf->indexes.begin() + splitPoint,
                          leaf->indexes.end());
  newLeaf->getData().assign(
      std::make_move_iterator(leaf->getData().begin() + splitPoint),
      std::make_move_iterator(leaf->getData().end()));

  // resize the original leaf
  leaf->indexes.resize((size_t)splitPoint);
  leaf->getData().resize((size_t)splitPoint);

  // update the next pointer
  newLeaf->next = leaf->next;
  leaf->next = newLeaf;

  // Promote the smallest index of the new leaf to the parent
  promoteToParent(leaf, newLeaf->indexes.front(), newLeaf);
}

// Promote the child to parent
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::promoteToParent(NodePtr left,
                                                  const IndexType &index,
                                                  NodePtr right) {
  if (left.get() == root.get()) {
    NodePtr newRoot = Node::createInternal();
    // set the indexes and children
    newRoot->indexes.emplace_back(index);
    newRoot->getChildren().emplace_back(std::move(left));
    newRoot->getChildren().emplace_back(std::move(right));
    // when internal node is generated, it has 1 index and 2 children
    // so num_children = num_indexes + 1
    root = std::move(newRoot);
    return;
  }

  NodePtr parent = findParent(left);

  // insert the index and the right child
  auto it =
      std::lower_bound(parent->indexes.begin(), parent->indexes.end(), index);
  auto idx = std::distance(parent->indexes.begin(), it);
  parent->indexes.insert(it, index);
  auto itChild = parent->getChildren().begin() + idx + 1;
  parent->getChildren().insert(itChild, std::move(right));

  // check if the parent is overflowing
  if (parent->indexes.size() >= maxIntChildren) {
    splitInternalNode(parent);
  }
}

// Split the internal node
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::splitInternalNode(NodePtr internal) {
  // create a new internal node
  NodePtr newInternal = Node::createInternal();
  // calculate the split point
  auto splitPoint =
      static_cast<typename std::vector<IndexType>::difference_type>(
          internal->indexes.size() / 2 + internal->indexes.size() % 2 - 1);

  // move half of the indexes and children to the new internal node
  newInternal->indexes.assign(internal->indexes.begin() + splitPoint + 1,
                              internal->indexes.end());
  newInternal->getChildren().assign(
      std::make_move_iterator(internal->getChildren().begin() + splitPoint + 1),
      std::make_move_iterator(internal->getChildren().end()));

  // get the middle index to be promoted
  IndexType promotedIndex = internal->indexes[(size_t)splitPoint];

  // resize the original internal
  internal->indexes.resize((size_t)splitPoint);
  internal->getChildren().resize((size_t)splitPoint + 1);

  // Promote the child to parent
  promoteToParent(internal, promotedIndex, std::move(newInternal));
}

// Rebalance the tree
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::delRebalance(NodePtr node, size_t idx) {
  // check if the leaf node is underflowing
  size_t minSize = maxLeafIdxes / 2;
  if (node->indexes.size() - 1 >= minSize) {
    removeFromNode(node, idx);
    return;
  }
  // handle underflow
  // handle level decrease triggered by underflow
  if (node == root) {
    removeFromNode(node, idx);
    if (!node->isLeaf && node->indexes.empty()) {
      root = std::move(node->getChildren()[0]);
    }
    return;
  }
  // get the parent, left and right siblings
  NodePtr parent = findParent(node);
  auto it = std::lower_bound(parent->indexes.begin(), parent->indexes.end(),
                             node->indexes.back());
  removeFromNode(node, idx);
  size_t idxParent = (size_t)std::distance(parent->indexes.begin(), it);
  NodePtr leftSibling =
      (idxParent > 0) ? parent->getChildren()[idxParent - 1] : nullptr;
  NodePtr rightSibling = (idxParent < parent->indexes.size())
                             ? parent->getChildren()[idxParent + 1]
                             : nullptr;
  if (leftSibling && leftSibling->indexes.size() > minSize) {
    borrowFromLeft(node, leftSibling, parent, idxParent - 1);
  } else if (rightSibling && rightSibling->indexes.size() > minSize) {
    borrowFromRight(node, rightSibling, parent, idxParent);
  } else if (leftSibling) {
    mergeNodes(leftSibling, node, parent, idxParent - 1);
  } else {
    mergeNodes(node, rightSibling, parent, idxParent);
  }
}

// Borrow a node from the left sibling
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::borrowFromLeft(NodePtr node,
                                                 NodePtr leftSibling,
                                                 NodePtr parent, size_t idx) {
  if (node->isLeaf) {
    node->indexes.insert(node->indexes.begin(), leftSibling->indexes.back());
    node->getData().insert(node->getData().begin(),
                           std::move(leftSibling->getData().back()));
    leftSibling->indexes.pop_back();
    leftSibling->getData().pop_back();
    // update the parent index
    parent->indexes[idx] = node->indexes.front();
  } else {
    node->indexes.insert(node->indexes.begin(), parent->indexes[idx]);
    node->getChildren().insert(node->getChildren().begin(),
                               std::move(leftSibling->getChildren().back()));
    leftSibling->indexes.pop_back();
    leftSibling->getChildren().pop_back();
    // update the parent index
    parent->indexes[idx] = leftSibling->indexes.back();
  }
}

// Borrow a node from the right sibling
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::borrowFromRight(NodePtr node,
                                                  NodePtr rightSibling,
                                                  NodePtr parent, size_t idx) {
  if (node->isLeaf) {
    node->indexes.emplace_back(rightSibling->indexes.front());
    node->getData().emplace_back(std::move(rightSibling->getData().front()));
    rightSibling->indexes.erase(rightSibling->indexes.begin());
    rightSibling->getData().erase(rightSibling->getData().begin());
    parent->indexes[idx] = rightSibling->indexes.front();
  } else {
    node->indexes.emplace_back(parent->indexes[idx]);
    node->getChildren().emplace_back(
        std::move(rightSibling->getChildren().front()));
    rightSibling->indexes.erase(rightSibling->indexes.begin());
    rightSibling->getChildren().erase(rightSibling->getChildren().begin());
    parent->indexes[idx] = rightSibling->indexes.front();
  }
}

// Merge the sibling nodes
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::mergeNodes(NodePtr left, NodePtr right,
                                             NodePtr parent, size_t idx) {
  if (left->isLeaf) {
    // merge the indexes and data
    left->indexes.insert(left->indexes.end(), right->indexes.begin(),
                         right->indexes.end());
    left->getData().insert(left->getData().end(),
                           std::make_move_iterator(right->getData().begin()),
                           std::make_move_iterator(right->getData().end()));
    // update the next pointer
    left->next = std::move(right->next);
  } else {
    // merge the indexes and children
    left->indexes.emplace_back(parent->indexes[idx]);
    left->indexes.insert(left->indexes.end(), right->indexes.begin(),
                         right->indexes.end());
    left->getChildren().insert(
        left->getChildren().end(),
        std::make_move_iterator(right->getChildren().begin()),
        std::make_move_iterator(right->getChildren().end()));
  }
  delRebalance(parent, idx);
}

// Insert a index-data pair into the B+ tree
template <typename IndexType, typename DataType>
bool BpTree<IndexType, DataType>::insert(const IndexType &index,
                                         const DataType &data) {
  // find the leaf node containing the index
  NodePtr leaf = findLeafNode(index)->getShared();

  bool isOverflow = (leaf->indexes.size() >= maxLeafIdxes);

  auto it = std::lower_bound(leaf->indexes.begin(), leaf->indexes.end(), index);
  if (it != leaf->indexes.end() && *it == index) {
    return false; // duplicate index
  }
  // insert the index and data (even if overflow, since we'll split later)
  auto idx = std::distance(leaf->indexes.begin(), it);
  leaf->indexes.insert(it, index);
  auto ptr = std::make_shared<DataType>(data);
  auto it_data = leaf->getData().begin() + idx;
  leaf->getData().insert(it_data, ptr);

  if (isOverflow)
    splitLeafNode(leaf, index, ptr);

  return true;
}

// Remove a node with index from the B+ tree
template <typename IndexType, typename DataType>
bool BpTree<IndexType, DataType>::erase(const IndexType &index) {
  if (!root)
    return false;
  // find the leaf node containing the index
  NodePtr leaf = findLeafNode(index);
  auto it = std::lower_bound(leaf->indexes.begin(), leaf->indexes.end(), index);
  if (it == leaf->indexes.end() || *it != index) {
    return false; // Index not found
  }
  // find the idx to remove
  auto idx = std::distance(leaf->indexes.begin(), it);
  if ((leaf == root) && (leaf->indexes.empty())) {
    root = Node::createLeaf();
    return true;
  }
  // del and rebalance the tree after deletion
  delRebalance(leaf, (size_t)idx);
  return true;
}

// Search for a specific index
template <typename IndexType, typename DataType>
std::shared_ptr<DataType>
BpTree<IndexType, DataType>::search(const IndexType &index) {
  // find the leaf node containing the index
  NodePtr leaf = findLeafNode(index);
  // find the index in the leaf node
  auto it = std::lower_bound(leaf->indexes.begin(), leaf->indexes.end(), index);
  if (it != leaf->indexes.end() && *it == index) {
    auto idx = std::distance(leaf->indexes.begin(), it);
    return leaf->getData()[(size_t)idx];
  }
  return nullptr;
}

// Get the minimum index in the B+ tree
template <typename IndexType, typename DataType>
IndexType BpTree<IndexType, DataType>::getMin() const {
  NodePtr current = getLeftmostLeaf();
  return current->indexes.front();
}

// Get the maximum index in the B+ tree
template <typename IndexType, typename DataType>
IndexType BpTree<IndexType, DataType>::getMax() const {
  NodePtr current = root;
  while (!current->isLeaf) {
    current = current->getChildren().back();
  }
  return current->indexes.back();
}

// Range query in the B+ tree
template <typename IndexType, typename DataType>
std::vector<std::shared_ptr<DataType>> BpTree<IndexType, DataType>::rangeQuery(
    const std::optional<IndexType> &minIndex,
    const std::optional<IndexType> &maxIndex, const bool &leftInclusive,
    const bool &rightInclusive) {
  std::vector<std::shared_ptr<DataType>> result;
  // Start from the leaf node containing minIndex or the leftmost leaf
  NodePtr current = minIndex ? findLeafNode(*minIndex) : getLeftmostLeaf();
  while (current) {
    size_t i = 0;
    // If minIndex is specified, find the starting point
    if (minIndex) {
      auto it = leftInclusive
                    ? std::lower_bound(current->indexes.begin(),
                                       current->indexes.end(), *minIndex)
                    : std::upper_bound(current->indexes.begin(),
                                       current->indexes.end(), *minIndex);
      i = static_cast<size_t>(std::distance(current->indexes.begin(), it));
    }
    // Iterate through the leaf node
    for (; i < current->indexes.size(); ++i) {
      // Check if current index is within the maxIndex and rightInclusive
      // condition
      if (maxIndex) {
        if ((rightInclusive && current->indexes[i] > *maxIndex) ||
            (!rightInclusive && current->indexes[i] >= *maxIndex))
          return result;
      }
      result.emplace_back(current->getData()[i]);
    }
    // Move to the next leaf node
    current = current->next;
  }
  return result;
}

// Count the number of indexes in the range
template <typename IndexType, typename DataType>
size_t BpTree<IndexType, DataType>::countRange(
    const std::optional<IndexType> &minIndex,
    const std::optional<IndexType> &maxIndex, const bool &leftInclusive,
    const bool &rightInclusive) {
  size_t count = 0;
  // Start from the leaf node containing minIndex or the leftmost leaf
  NodePtr current = minIndex ? findLeafNode(*minIndex) : getLeftmostLeaf();
  while (current) {
    size_t i = 0;
    // If minIndex is specified, find the starting point
    if (minIndex) {
      auto it = leftInclusive
                    ? std::lower_bound(current->indexes.begin(),
                                       current->indexes.end(), *minIndex)
                    : std::upper_bound(current->indexes.begin(),
                                       current->indexes.end(), *minIndex);
      i = static_cast<size_t>(std::distance(current->indexes.begin(), it));
    }
    // Iterate through the leaf node
    for (; i < current->indexes.size(); ++i) {
      // Check if current index is within the maxIndex and rightInclusive
      // condition
      if (maxIndex) {
        if ((rightInclusive && current->indexes[i] > *maxIndex) ||
            (!rightInclusive && current->indexes[i] >= *maxIndex))
          return count;
      }
      ++count;
    }
    // Move to the next leaf node
    current = current->next;
  }
  return count;
}
// Utility function to print the B+ Tree
template <typename IndexType, typename DataType>
void BpTree<IndexType, DataType>::printTree() const {
  if (!root)
    return;
  std::vector<NodePtr> currentLevel;
  std::vector<NodePtr> nextLevel;
  currentLevel.emplace_back(root);
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
        for (size_t i = 0; i < node->getChildren().size(); ++i) {
          nextLevel.emplace_back(node->getChildren()[i]);
        }
      }
    }
    std::cout << std::endl;
    currentLevel = nextLevel;
    nextLevel.clear();
  }
}

template <typename IndexType, typename DataType>
DataType& BpTree<IndexType, DataType>::operator[](const IndexType& index) {
  auto existingData = search(index);
  if (existingData) {
    // If the index exists, return a reference to the existing data
    return *existingData;
  } else {
    // If the index doesn't exist, insert a new element with default-constructed data
    DataType newData{};
    insert(index, newData);
    
    // Search again to get the pointer to the newly inserted data
    auto insertedData = search(index);
    return *insertedData;
  }
}

#endif
