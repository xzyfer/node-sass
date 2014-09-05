#include <deque>
#include <iostream>

#ifndef SASS_AST
#include "ast.hpp"
#endif

#include "node.hpp"


namespace Sass {

  
  using namespace std;


  /*
   This is for ports of functions in the Sass:Util module.
   */
  

	/*
    # Return a Node collection of all possible paths through the given Node collection of Node collections.
    #
    # @param arrs [NodeCollection<NodeCollection<Node>>]
    # @return [NodeCollection<NodeCollection<Node>>]
    #
    # @example
    #   paths([[1, 2], [3, 4], [5]]) #=>
    #     # [[1, 3, 5],
    #     #  [2, 3, 5],
    #     #  [1, 4, 5],
    #     #  [2, 4, 5]]
  */
	Node paths(const Node& arrs, Context& ctx);
  
  
  /*
  This class is a default implementation of a Node comparator that can be passed to the lcs function below.
  It uses operator== for equality comparision. It then returns one if the Nodes are equal.
  */
  class DefaultLcsComparator {
  public:
  	bool operator()(const Node& one, const Node& two, Node& out) const {
    	// TODO: Is this the correct C++ interpretation?
      // block ||= proc {|a, b| a == b && a}
      if (one == two) {
      	out = one;
        return true;
      }

      return false;
    }
  };

  
	typedef vector<vector<int> > LCSTable;
  
  
  /*
  This is the equivalent of ruby's Sass::Util.lcs_backtrace.
  
  # Computes a single longest common subsequence for arrays x and y.
  # Algorithm from http://en.wikipedia.org/wiki/Longest_common_subsequence_problem#Reading_out_an_LCS
	*/
  template<typename ComparatorType>
  Node lcs_backtrace(const LCSTable& c, const Node& x, const Node& y, int i, int j, const ComparatorType& comparator) {
  	if (i == 0 || j == 0) {
    	return Node::createCollection();
    }
    
  	NodeDeque& xChildren = *(x.collection());
    NodeDeque& yChildren = *(y.collection());

    Node compareOut = Node::createNil();
    if (comparator(xChildren[i], yChildren[j], compareOut)) {
      Node result = lcs_backtrace(c, x, y, i - 1, j - 1, comparator);
      result.collection()->push_back(compareOut);
      return result;
    }
    
    if (c[i][j - 1] > c[i - 1][j]) {
    	return lcs_backtrace(c, x, y, i, j - 1, comparator);
    }
    
    return lcs_backtrace(c, x, y, i - 1, j, comparator);
  }
  

  /*
  This is the equivalent of ruby's Sass::Util.lcs_table.
  
  # Calculates the memoization table for the Least Common Subsequence algorithm.
  # Algorithm from http://en.wikipedia.org/wiki/Longest_common_subsequence_problem#Computing_the_length_of_the_LCS
  */
  template<typename ComparatorType>
  void lcs_table(const Node& x, const Node& y, const ComparatorType& comparator, LCSTable& out) {
  	NodeDeque& xChildren = *(x.collection());
    NodeDeque& yChildren = *(y.collection());

  	LCSTable c(xChildren.size(), vector<int>(yChildren.size()));
    
    // These shouldn't be necessary since the vector will be initialized to 0 already.
    // x.size.times {|i| c[i][0] = 0}
    // y.size.times {|j| c[0][j] = 0}

    for (int i = 1; i < xChildren.size(); i++) {
    	for (int j = 1; j < yChildren.size(); j++) {
        Node compareOut = Node::createNil();

      	if (comparator(xChildren[i], yChildren[j], compareOut)) {
        	c[i][j] = c[i - 1][j - 1] + 1;
        } else {
        	c[i][j] = max(c[i][j - 1], c[i - 1][j]);
        }
      }
    }

    out = c;
  }


  /*
  This is the equivalent of ruby's Sass::Util.lcs.
  
  # Computes a single longest common subsequence for `x` and `y`.
  # If there are more than one longest common subsequences,
  # the one returned is that which starts first in `x`.

  # @param x [NodeCollection]
  # @param y [NodeCollection]
  # @comparator An equality check between elements of `x` and `y`.
  # @return [NodeCollection] The LCS

  http://en.wikipedia.org/wiki/Longest_common_subsequence_problem
  */
  template<typename ComparatorType>
  Node lcs(const Node& x, const Node& y, const ComparatorType& comparator, Context& ctx) {
    Node newX = x.clone(ctx);
    newX.collection()->push_front(Node::createNil());
    
    Node newY = x.clone(ctx);
    newY.collection()->push_front(Node::createNil());
    
    LCSTable table;
    lcs_table(newX, newY, comparator, table);
    
		return lcs_backtrace(table, newX, newY, newX.collection()->size() - 1, newY.collection()->size() - 1, comparator);
  }

}