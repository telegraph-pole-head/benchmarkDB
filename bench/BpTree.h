#ifndef PROJECT_DB_BPTREE_H
#define PROJECT_DB_BPTREE_H

#include <memory>
#include <optional>
#include <variant>
#include <vector>

template <typename IndexType, typename DataType> class BpTree {
private:
  class Node : public std::enable_shared_from_this<Node> {
  public:
    typedef std::vector<std::shared_ptr<DataType>>
        DataContent; // For leaf nodes
    typedef std::vector<std::shared_ptr<Node>>
        ChildrenContent; // For internal nodes

    bool isLeaf; // True if node is a leaf, false if node is an internal node
    std::vector<IndexType> indexes; // the index of the node
    std::shared_ptr<Node> next;     // the next leaf node
    std::variant<DataContent, ChildrenContent> content;

    Node(bool isLeaf) : isLeaf(isLeaf), indexes(), next(nullptr) {
      if (isLeaf) {
        content = DataContent();
      } else {
        content = ChildrenContent();
      }
    }
    // create a new node
    static std::shared_ptr<Node> createLeaf() {
      return std::make_shared<Node>(true);
    }
    static std::shared_ptr<Node> createInternal() {
      return std::make_shared<Node>(false);
    }
    // for shared pointer
    std::shared_ptr<Node> getShared() { return this->shared_from_this(); }
    // get the reference to the data or children
    std::vector<std::shared_ptr<DataType>> &getData() {
      return std::get<DataContent>(content);
    }
    std::vector<std::shared_ptr<Node>> &getChildren() {
      return std::get<ChildrenContent>(content);
    }
  };

  using NodePtr = std::shared_ptr<Node>;
  NodePtr root;
  size_t maxIntChildren; // Limiting #of children for an internal Node
  size_t maxLeafIdxes;   // Limiting #of indexes for a leaf Node

  // Find the leaf node for index
  NodePtr findLeafNode(const IndexType &index) const;
  // Find parent of a node
  NodePtr findParent(NodePtr &child) const;
  // Get the leftmost leaf node
  NodePtr getLeftmostLeaf() const;
  // Remove the index and data/children from the node
  void removeFromNode(NodePtr node, size_t pos);
  // Split the leaf node
  void splitLeafNode(NodePtr leaf, const IndexType &index,
                     std::shared_ptr<DataType> data);
  // Promote the child to parent
  void promoteToParent(NodePtr left, const IndexType &index, NodePtr right);
  // Split the internal node
  void splitInternalNode(NodePtr internal);
  // Rebalance the tree
  void delRebalance(NodePtr node, size_t idx);
  // Borrow a node from the sibling
  void borrowFromLeft(NodePtr node, NodePtr leftSibling, NodePtr parent,
                      size_t idx);
  void borrowFromRight(NodePtr node, NodePtr rightSibling, NodePtr parent,
                       size_t idx);
  // Merge the nodes
  void mergeNodes(NodePtr left, NodePtr right, NodePtr parent, size_t idx);

public:
  BpTree()
      : root(std::make_shared<Node>(true)), maxIntChildren(8), maxLeafIdxes(7) {
  }

  BpTree(size_t order)
      : root(std::make_shared<Node>(true)), maxIntChildren(order),
        maxLeafIdxes(order - 1) {}

  BpTree(size_t maxIntChildren, size_t maxLeafIdxes)
      : root(std::make_shared<Node>(true)), maxIntChildren(maxIntChildren),
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
  bool erase(const IndexType &index);

  /**
   * @brief         search for a specific index
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @param         index
   * @return        std::shared_ptr<DataType>, nullptr if not found
   */
  std::shared_ptr<DataType> search(const IndexType &index);

  /**
   * @brief         Get the minimum index in the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @return        IndexType
   */
  IndexType getMin() const;

  /**
   * @brief         Get the maximum index in the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @return        IndexType
   */
  IndexType getMax() const;

  /**
   * @brief         Range query in the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   * @param         minIndex , if input is std::nullopt, start from the leftmost
   * @param         maxIndex , if input is std::nullopt, end at the rightmost
   * @return        std::vector<std::shared_ptr<DataType>>
   */
  std::vector<std::shared_ptr<DataType>>
  rangeQuery(const std::optional<IndexType> &minIndex,
             const std::optional<IndexType> &maxIndex,
             const bool &leftInclusive = true,
             const bool &rightInclusive = true);

  /**
   * @brief         Count the number of indexes in the range
   *
   * @param         minIndex , if input is std::nullopt, start from the leftmost
   * @param         maxIndex , if input is std::nullopt, end at the rightmost
   * @return        size_t
   */
  size_t countRange(const std::optional<IndexType> &minIndex,
                    const std::optional<IndexType> &maxIndex,
                    const bool &leftInclusive = true,
                    const bool &rightInclusive = true);

  /**
   * @brief         Print the B+ tree
   *
   * @tparam        IndexType
   * @tparam        DataType
   */
  void printTree() const;

  /**
   * @brief Overload the [] operator for insert, search, and update
   *
   * @param index The index to search for
   * @return A reference to the DataType associated with the index
   */
  DataType& operator[](const IndexType& index);
};

#include "BpTreeImpl.h" // Include the implementation

#endif // PROJECT_DB_BPTREE_H
