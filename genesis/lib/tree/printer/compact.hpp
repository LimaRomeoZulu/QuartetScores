#ifndef GENESIS_TREE_PRINTER_COMPACT_H_
#define GENESIS_TREE_PRINTER_COMPACT_H_

/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2016 Lucas Czech

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact:
    Lucas Czech <lucas.czech@h-its.org>
    Exelixis Lab, Heidelberg Institute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

/**
 * @brief
 *
 * @file
 * @ingroup tree
 */

#include <iosfwd>
#include <functional>
#include <string>

namespace genesis {
namespace tree {

// =================================================================================================
//     Forward Declarations
// =================================================================================================

class Tree;
class TreeNode;
class TreeEdge;
class TreeLink;

// =================================================================================================
//     Printer Compact
// =================================================================================================

/**
 * @brief
 */
class PrinterCompact
{
public:

    // -------------------------------------------------------------------------
    //     Constructor and Rule of Five
    // -------------------------------------------------------------------------

    PrinterCompact()  = default;
    ~PrinterCompact() = default;

    PrinterCompact( PrinterCompact const& ) = default;
    PrinterCompact( PrinterCompact&& )      = default;

    PrinterCompact& operator= ( PrinterCompact const& ) = default;
    PrinterCompact& operator= ( PrinterCompact&& )      = default;

    // -------------------------------------------------------------------------
    //     Print
    // -------------------------------------------------------------------------

    void print (
        std::ostream&   out,
        Tree const& tree,
        std::function<std::string (
            TreeNode const& node,
            TreeEdge const& edge
        )> const print_line
    );

    std::string print (
        Tree const& tree,
        std::function<std::string (
            TreeNode const& node,
            TreeEdge const& edge
        )> const print_line
    );

    std::string print ( Tree const& tree );

    // std::string operator() ( Tree const& tree );

};

} // namespace tree
} // namespace genesis

#endif // include guard
