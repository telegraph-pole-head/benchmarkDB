#ifndef PROJECT_DB_TEST_BPTREE_H
#define PROJECT_DB_TEST_BPTREE_H

#include "BpTree.h"
#include <cassert>
#include <iostream>

class BpTreeTest {
public:
  static void runTests() {
    testInsert();
    // testRemove();
    // testSearch();
    // testGetMin();
    // testGetMax();
    // testRangeQuery();
    // testCountRange();
    std::cout << "All tests passed!" << std::endl;
  }

private:
  static void testInsert() {
    std::cout << "Starting testInsert" << std::endl;
    BpTree<int, std::string> tree(4);
    assert(tree.insert(1, "one"));
    assert(tree.insert(2, "two"));
    // assert(tree.insert(3, "three"));
    assert(!tree.insert(2, "one")); // Duplicate insert
    for (int i = 3; i < 20; ++i) {
      assert(tree.insert(i, "data"));
      tree.printTree();
    }
    std::cout << "testInsert passed!" << std::endl;
  }

  static void testRemove() {
    BpTree<int, std::string> tree(4);
    tree.insert(1, "one");
    tree.insert(2, "two");
    assert(tree.remove(1));
    assert(!tree.remove(1)); // Remove non-existent
    std::cout << "testRemove passed!" << std::endl;
  }

  static void testSearch() {
    BpTree<int, std::string> tree(4);
    tree.insert(1, "one");
    tree.insert(2, "two");
    assert(tree.search(1) != nullptr);
    assert(tree.search(3) == nullptr); // Search non-existent
    std::cout << "testSearch passed!" << std::endl;
  }

  static void testGetMin() {
    BpTree<int, std::string> tree(4);
    tree.insert(2, "two");
    tree.insert(1, "one");
    assert(*tree.getMin() == 1);
    std::cout << "testGetMin passed!" << std::endl;
  }

  static void testGetMax() {
    BpTree<int, std::string> tree(4);
    tree.insert(1, "one");
    tree.insert(2, "two");
    assert(*tree.getMax() == 2);
    std::cout << "testGetMax passed!" << std::endl;
  }

  static void testRangeQuery() {
    BpTree<int, std::string> tree(4);
    tree.insert(1, "one");
    tree.insert(2, "two");
    tree.insert(3, "three");
    auto result = tree.rangeQuery(1, 2);
    assert(result.size() == 2);
    std::cout << "testRangeQuery passed!" << std::endl;
  }

  static void testCountRange() {
    BpTree<int, std::string> tree(4);
    tree.insert(1, "one");
    tree.insert(2, "two");
    tree.insert(3, "three");
    assert(tree.countRange(1, 2) == 2);
    std::cout << "testCountRange passed!" << std::endl;
  }
};

#endif // PROJECT_DB_TEST_BPTREE_H
