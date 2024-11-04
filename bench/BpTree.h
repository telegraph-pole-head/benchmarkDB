#ifndef PROJECT_DB_BPTREE_H
#define PROJECT_DB_BPTREE_H

#include <memory>
#include <optional>
#include <vector>

template <typename IndexType, typename DataType> class BpTree {
private:
  struct Node {
    bool isLeaf; // True if node is a leaf, false if node is an internal node
    std::vector<IndexType> indexes; // the index of the node
    std::unique_ptr<Node> next;     // the next leaf node
    union ptrs {
      std::vector<std::unique_ptr<Node>> children;
      std::vector<std::unique_ptr<DataType>> data; // Only used in leaf nodes
      ptrs() {}
      ~ptrs() {}
    } ptrChildrenOrData;

    Node(bool isLeaf) : isLeaf(isLeaf) {
      new (&indexes) std::vector<IndexType>();
      next = nullptr;
      if (isLeaf) {
        new (&ptrChildrenOrData.data) std::vector<std::unique_ptr<DataType>>();
      } else {
        new (&ptrChildrenOrData.children) std::vector<std::unique_ptr<Node>>();
      }
    }

    ~Node() {
      if (isLeaf) {
        ptrChildrenOrData.data.~vector();
      } else {
        ptrChildrenOrData.children.~vector();
      }
    }
  };

  using NodePtr = std::unique_ptr<Node>;
  NodePtr root;
  size_t maxIntChildren; // Limiting #of children for an internal Node
  size_t maxLeafIdxes;   // Limiting #of indexes for a leaf Node

  // Find the leaf node for index
  Node *findLeafNode(const IndexType &index);
  // Find parent of a node
  Node *findParent(Node *child);
  // Get the leftmost leaf node
  Node *getLeftmostLeaf();
  // Remove the index and data/children from the node
  void removeFromNode(Node *node, size_t pos);
  // Split the leaf node
  void splitLeafNode(Node *leaf, const IndexType &index,
                     std::unique_ptr<DataType> data);
  // Promote the child to parent
  void promoteToParent(Node *left, const IndexType &index, NodePtr right);
  // Split the internal node
  void splitInternalNode(Node *internal);
  // Rebalance the tree
  void rebalance(Node *node);
  // Borrow a node from the sibling
  void borrowFromLeft(Node *node, Node *leftSibling, Node *parent, size_t idx);
  void borrowFromRight(Node *node, Node *rightSibling, Node *parent,
                       size_t idx);
  // Merge the nodes
  void mergeNodes(Node *left, Node *right, Node *parent, size_t idx);

public:
  BpTree()
      : root(std::make_unique<Node>(true)), maxIntChildren(8), maxLeafIdxes(7) {
  }

  BpTree(size_t order)
      : root(std::make_unique<Node>(true)), maxIntChildren(order),
        maxLeafIdxes(order - 1) {}

  BpTree(size_t maxIntChildren, size_t maxLeafIdxes)
      : root(std::make_unique<Node>(true)), maxIntChildren(maxIntChildren),
        maxLeafIdxes(maxLeafIdxes) {}

  /**
   * @brief         Insert a index-data pair into the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @param         index
   * @param         data
   * @return        true if the insertion is successfu
   * @return        false if the index already exists
   */
  bool insert(const IndexType &index, const DataType &data);

  /**
   * @brief         Remove a node with index from the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @param         index
   * @return        true if the removal is successful
   * @return        false if the index is not found
   */
  bool remove(const IndexType &index);

  /**
   * @brief         search for a specific index
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @param         index
   * @return        std::unique_ptr<DataType>, nullptr if not found
   */
  std::unique_ptr<DataType> search(const IndexType &index);

  /**
   * @brief         Get the minimum index in the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @return        std::unique_ptr<IndexType>
   */
  std::unique_ptr<IndexType> getMin();

  /**
   * @brief         Get the maximum index in the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @return        std::unique_ptr<IndexType>
   */
  std::unique_ptr<IndexType> getMax();

  /**
   * @brief         Range query in the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @param         minIndex , if input is std::nullopt, start from the leftmost
   * @param         maxIndex , if input is std::nullopt, end at the rightmost
   * @return        std::vector<std::unique_ptr<DataType>>
   */
  std::vector<std::unique_ptr<DataType>>
  rangeQuery(const std::optional<IndexType> &minIndex,
             const std::optional<IndexType> &maxIndex);

  /**
   * @brief         Count the number of indexes in the range
   *
   * @param         minIndex , if input is std::nullopt, start from the leftmost
   * @param         maxIndex , if input is std::nullopt, end at the rightmost
   * @return        size_t
   */
  size_t countRange(const std::optional<IndexType> &minIndex,
                    const std::optional<IndexType> &maxIndex);

  /**
   * @brief         Print the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   */
  void printTree() const;
};

#include "BpTree.tpp" // Include the implementation

#endif // PROJECT_DB_BPTREE_H
