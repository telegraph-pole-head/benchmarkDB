#ifndef PROJECT_DB_TEST_BPTREE_H
#define PROJECT_DB_TEST_BPTREE_H

#include "BpTree.h"
#include <cassert>
#include <iostream>
#include <memory>

class BpTreeTest {
public:
  static void runTests() {
    testInsert();
    testRemove();
    testSearch();
    testGetMinMax();
    testRangeQuery();
    testCountRange();
    std::cout << "All tests passed!" << std::endl;
  }

private:
  static void testInsert() {
    std::cout << "Starting testInsert" << std::endl;
    BpTree<int, std::string> tree(3);
    assert(tree.insert(1, "one"));
    assert(tree.insert(2, "two"));
    // assert(tree.insert(3, "three"));
    assert(!tree.insert(2, "one")); // Duplicate insert
    for (int i = 3; i < 20; ++i) {
      assert(tree.insert(i, "d"));
      tree.printTree();
    }
    std::cout << "testInsert passed!" << std::endl;
  }

  static void testRemove() {
    BpTree<int, std::string> tree(3);
    for (int i = 1; i < 20; ++i) {
      assert(tree.insert(i, "d"));
    }
    assert(tree.remove(5));
    assert(!tree.remove(5)); // Remove non-existent
    tree.printTree();
    assert(tree.remove(16)); // test borrow
    tree.printTree();
    for (int i = 1; i < 20; ++i) {
      if (i != 5 && i != 16) {
        assert(tree.remove(i));
        tree.printTree();
      }
    }
    std::cout << "testRemove passed!" << std::endl;
  }

  static void testSearch() {
    BpTree<int, std::string> tree(3);
    for (int i = 1; i < 20; ++i) {
      assert(tree.insert(i, "d"));
    }
    std::shared_ptr<std::string> dataIn5 = tree.search(5);
    assert(dataIn5 != nullptr);
    // update the data
    *dataIn5 = "five";
    assert(*tree.search(5) == "five");
    assert(tree.remove(11));
    assert(tree.search(11) == nullptr); // Search non-existent
    std::cout << "testSearch passed!" << std::endl;
  }

  static void testGetMinMax() {
    BpTree<int, std::string> tree(3);
    for (int i = 1; i < 50; ++i) {
      assert(tree.insert(i, "d"));
    }
    assert(tree.getMin() == 1);
    assert(tree.getMax() == 49);
    std::cout << "testGetMinMax passed!" << std::endl;
  }

  static void testRangeQuery() {
    BpTree<int, std::string> tree(3);
    for (int i = -11; i < 20; ++i) {
      assert(tree.insert(i, std::to_string(i)));
    }
    auto result = tree.rangeQuery(-2, 2);
    assert(result.size() == 5);
    for (int i = 0; i < 5; ++i) {
      assert(*result[i] == std::to_string(i - 2));
    }
    std::cout << "testRangeQuery passed!" << std::endl;
  }

  static void testCountRange() {
    BpTree<int, std::string> tree(3);
    for (int i = -1; i < 20; ++i) {
      assert(tree.insert(i, std::to_string(i)));
    }
    assert(tree.countRange(-1, 3) == 5);
    assert(tree.countRange(std::nullopt, 3, true, false) == 4);
    assert(tree.countRange(-1, std::nullopt) == 21);
    std::cout << "testCountRange passed!" << std::endl;
  }
};

#endif // PROJECT_DB_TEST_BPTREE_H
