# HTML review checklist: 4.9 Tabular data

Generated from <https://html.spec.whatwg.org/> on 2026-10-31 by `tools/spec_checklist.py`: 42 algorithms. Regenerating keeps each section's status line and the `>` notes under it, and each algorithm's checkbox and the `>` notes under it. `python tools/spec_status.py html tables` reports where each algorithm is implemented and tested.

Reviewing a section: find each algorithm's implementation (its `@see` link), label its steps as below (`// 1.`, `// 1a.`, `// 1a.i.` with the step's text), fix or record divergences (a step left out on purpose as `// 3. (Skipped: <reason>.)` with a `DIFFERENCES.md` entry, a bug as a `// FIXME:`, a `[!shouldfail]` test and a `KNOWN-ISSUES.md` entry), tag the tests that cover it (`[html:<anchor>]`), tick it, and set the section's status. A section without algorithms still has definitions and requirements to check against the code: note where they live in `>` lines under its status, and tag their tests with the definition's or the section's anchor.

Statuses: `unreviewed`, `in review`, `reviewed (<date>)`, `n/a (<reason>)`. An algorithm is ticked once it's reviewed; one that doesn't apply is ticked with a `> n/a: <reason>` note. `python tools/spec_status.py html --tick <section anchor>` ticks a section's algorithms and sets its status.

### [4.9 Tabular data](https://html.spec.whatwg.org/#tables)

Status: reviewed (2026-11-01)
> Headings only.

#### [4.9.1 The table element](https://html.spec.whatwg.org/#the-table-element)

Status: reviewed (2026-11-01)
> `HTMLTableElement`; `rows` is `HTMLTableRowsCollection`, and table elements are created by `CreateTableElement`. Its presentational hints are Rendering's.

- [x] [create a table element](https://html.spec.whatwg.org/#create-a-table-element) — To create a table element given a table element tableElement and a string localName, return the result of creating an element given tableElement's node document, localName, and the HTML namespace.

- [x] [HTMLTableElement.caption](https://html.spec.whatwg.org/#dom-table-caption) — The caption getter steps are to return the first caption element child of this, if any; otherwise null.

- [x] [The caption setter steps](https://html.spec.whatwg.org/#dom-table-caption) — The caption setter steps are:
  - 1. Remove the first caption element child of this, if any.
  - 2. If the given value is not null, then insert it as the first node of this.

- [x] [HTMLTableElement.createCaption()](https://html.spec.whatwg.org/#dom-table-createcaption) — The createCaption() method steps are:
  - 1. If this has a caption element child, then return the first such element.
  - 2. Let caption be the result of creating a table element given this and "caption".
  - 3. Insert caption as the first node of this.
  - 4. Return caption.

- [x] [HTMLTableElement.deleteCaption()](https://html.spec.whatwg.org/#dom-table-deletecaption) — The deleteCaption() method steps are to remove the first caption element child of this, if any.

- [x] [HTMLTableElement.tHead](https://html.spec.whatwg.org/#dom-table-thead) — The tHead getter steps are to return the first thead element child of this, if any; otherwise null.

- [x] [The tHead setter steps](https://html.spec.whatwg.org/#dom-table-thead) — The tHead setter steps are:
  - 1. If the given value is neither null nor a thead element, then throw a "HierarchyRequestError" DOMException.
  - 2. Remove the first thead element child of this, if any.
  - 3. If the given value is not null, then insert it immediately before the first element child of this that is neither a caption element nor a colgroup element, if any; otherwise insert it at the end of this.

- [x] [HTMLTableElement.createTHead()](https://html.spec.whatwg.org/#dom-table-createthead) — The createTHead() method steps are:
  - 1. If this has a thead element child, then return the first such element.
  - 2. Let thead be the result of creating a table element given this and "thead".
  - 3. Insert thead immediately before the first element child of this that is neither a caption element nor a colgroup element, if any; otherwise insert thead at the end of this.
  - 4. Return thead.

- [x] [HTMLTableElement.deleteTHead()](https://html.spec.whatwg.org/#dom-table-deletethead) — The deleteTHead() method steps are to remove the first thead element child of this, if any.

- [x] [HTMLTableElement.tFoot](https://html.spec.whatwg.org/#dom-table-tfoot) — The tFoot getter steps are to return the first tfoot element child of this, if any; otherwise null.

- [x] [The tFoot setter steps](https://html.spec.whatwg.org/#dom-table-tfoot) — The tFoot setter steps are:
  - 1. If the given value is neither null nor a tfoot element, then throw a "HierarchyRequestError" DOMException.
  - 2. Remove the first tfoot element child of this, if any.
  - 3. If the given value is not null, then insert it at the end of this.

- [x] [HTMLTableElement.createTFoot()](https://html.spec.whatwg.org/#dom-table-createtfoot) — The createTFoot() method steps are:
  - 1. If this has a tfoot element child, then return the first such element.
  - 2. Let tfoot be the result of creating a table element given this and "tfoot".
  - 3. Insert tfoot at the end of this.
  - 4. Return tfoot.

- [x] [HTMLTableElement.deleteTFoot()](https://html.spec.whatwg.org/#dom-table-deletetfoot) — The deleteTFoot() method steps are to remove the first tfoot element child of this, if any.

- [x] [HTMLTableElement.tBodies](https://html.spec.whatwg.org/#dom-table-tbodies) — The tBodies getter steps are to return an HTMLCollection rooted at this, whose filter matches only tbody elements that are children of this.

- [x] [HTMLTableElement.createTBody()](https://html.spec.whatwg.org/#dom-table-createtbody) — The createTBody() method steps are:
  - 1. Let tbody be the result of creating a table element given this and "tbody".
  - 2. Insert tbody immediately after the last tbody element child of this, if any; otherwise insert tbody at the end of this.
  - 3. Return tbody.

- [x] [HTMLTableElement.rows](https://html.spec.whatwg.org/#dom-table-rows) — The rows getter steps are to return an HTMLCollection rooted at this, whose filter matches only tr elements that are either children of this, or children of thead, tbody, or tfoot elements that are themselves children of this. The elements in the collection must be ordered such that those elements whose parent is a thead are included first, in tree order, followed by those elements whose parent is either this or a tbody element, again in tree order, followed finally by those elements whose parent is a tfoot element, still in tree order.

- [x] [HTMLTableElement.insertRow(index)](https://html.spec.whatwg.org/#dom-table-insertrow) — The insertRow(index) method steps are:
  - 1. If index is less than −1 or greater than the number of elements in the rows collection, then throw an "IndexSizeError" DOMException.
  - 2. Let tr be the result of creating a table element given this and "tr".
  - 3. If the rows collection has zero elements in it, and this has no tbody elements in it:
    - 3a. Let tbody be the result of creating a table element given this and "tbody".
    - 3b. Append tr to tbody.
    - 3c. Append tbody to this.
  - 4. Otherwise, if the rows collection has zero elements in it, then append tr to the last tbody element in this.
  - 5. Otherwise, if index is −1 or equal to the number of items in the rows collection, then append tr to the parent of the last tr element in the rows collection.
  - 6. Otherwise, insert tr immediately before the indexth tr element in the rows collection, in the same parent.
  - 7. Return tr.

- [x] [HTMLTableElement.deleteRow(index)](https://html.spec.whatwg.org/#dom-table-deleterow) — The deleteRow(index) method steps are:
  - 1. If index is less than −1 or greater than or equal to the number of elements in the rows collection, then throw an "IndexSizeError" DOMException.
  - 2. If index is −1, then remove the last element in the rows collection from its parent, or do nothing if the rows collection is empty.
  - 3. Otherwise, remove the indexth element in the rows collection from its parent.

##### [4.9.1.1 Techniques for describing tables](https://html.spec.whatwg.org/#table-descriptions-techniques)

Status: n/a (author conformance)

##### [4.9.1.2 Techniques for table design](https://html.spec.whatwg.org/#table-layout-techniques)

Status: n/a (author conformance)

#### [4.9.2 The caption element](https://html.spec.whatwg.org/#the-caption-element)

Status: reviewed (2026-11-01)
> `HTMLTableCaptionElement`; its obsolete `align` is reflected.

#### [4.9.3 The colgroup element](https://html.spec.whatwg.org/#the-colgroup-element)

Status: reviewed (2026-11-01)
> `HTMLTableColElement`: `span` is reflected, clamped to 1 to 1000.
<!-- definitions: attr-colgroup-span -->

#### [4.9.4 The col element](https://html.spec.whatwg.org/#the-col-element)

Status: reviewed (2026-11-01)
> `HTMLTableColElement`, as for `colgroup`.
<!-- definitions: attr-col-span -->

#### [4.9.5 The tbody element](https://html.spec.whatwg.org/#the-tbody-element)

Status: reviewed (2026-11-01)
> `HTMLTableSectionElement`.

- [x] [HTMLTableSectionElement.rows](https://html.spec.whatwg.org/#dom-tbody-rows) — The rows attribute must return an HTMLCollection rooted at this element, whose filter matches only tr elements that are children of this element.

- [x] [HTMLTableSectionElement.insertRow(index)](https://html.spec.whatwg.org/#dom-tbody-insertrow) — The insertRow(index) method must act as follows:
  - 1. If index is less than −1 or greater than the number of elements in the rows collection, throw an "IndexSizeError" DOMException.
  - 2. Let table row be the result of creating an element given this element's node document, "tr", and the HTML namespace.
  - 3. If index is −1 or equal to the number of items in the rows collection, then append table row to this element.
  - 4. Otherwise, insert table row as a child of this element, immediately before the indexth tr element in the rows collection.
  - 5. Return table row.

- [x] [HTMLTableSectionElement.deleteRow(index)](https://html.spec.whatwg.org/#dom-tbody-deleterow) — The deleteRow(index) method must, when invoked, act as follows:
  - 1. If index is less than −1 or greater than or equal to the number of elements in the rows collection, then throw an "IndexSizeError" DOMException.
  - 2. If index is −1, then remove the last element in the rows collection from this element, or do nothing if the rows collection is empty.
  - 3. Otherwise, remove the indexth element in the rows collection from this element.

#### [4.9.6 The thead element](https://html.spec.whatwg.org/#the-thead-element)

Status: reviewed (2026-11-01)
> `HTMLTableSectionElement`.

#### [4.9.7 The tfoot element](https://html.spec.whatwg.org/#the-tfoot-element)

Status: reviewed (2026-11-01)
> `HTMLTableSectionElement`.

#### [4.9.8 The tr element](https://html.spec.whatwg.org/#the-tr-element)

Status: reviewed (2026-11-01)
> `HTMLTableRowElement`.

- [x] [HTMLTableRowElement.rowIndex](https://html.spec.whatwg.org/#dom-tr-rowindex) — The rowIndex attribute must, if this element has a parent table element, or a parent tbody, thead, or tfoot element and a grandparent table element, return the index of this tr element in that table element's rows collection. If there is no such table element, then the attribute must return −1.

- [x] [HTMLTableRowElement.sectionRowIndex](https://html.spec.whatwg.org/#dom-tr-sectionrowindex) — The sectionRowIndex attribute must, if this element has a parent table, tbody, thead, or tfoot element, return the index of the tr element in the parent element's rows collection (for tables, that's HTMLTableElement's rows collection; for table sections, that's HTMLTableSectionElement's rows collection). If there is no such parent element, then the attribute must return −1.

- [x] [HTMLTableRowElement.cells](https://html.spec.whatwg.org/#dom-tr-cells) — The cells attribute must return an HTMLCollection rooted at this tr element, whose filter matches only td and th elements that are children of the tr element.

- [x] [HTMLTableRowElement.insertCell(index)](https://html.spec.whatwg.org/#dom-tr-insertcell) — The insertCell(index) method must act as follows:
  - 1. If index is less than −1 or greater than the number of elements in the cells collection, then throw an "IndexSizeError" DOMException.
  - 2. Let table cell be the result of creating an element given this tr element's node document, "td", and the HTML namespace.
  - 3. If index is equal to −1 or equal to the number of items in cells collection, then append table cell to this tr element.
  - 4. Otherwise, insert table cell as a child of this tr element, immediately before the indexth td or th element in the cells collection.
  - 5. Return table cell.

- [x] [HTMLTableRowElement.deleteCell(index)](https://html.spec.whatwg.org/#dom-tr-deletecell) — The deleteCell(index) method must act as follows:
  - 1. If index is less than −1 or greater than or equal to the number of elements in the cells collection, then throw an "IndexSizeError" DOMException.
  - 2. If index is −1, then remove the last element in the cells collection from its parent, or do nothing if the cells collection is empty.
  - 3. Otherwise, remove the indexth element in the cells collection from its parent.

#### [4.9.9 The td element](https://html.spec.whatwg.org/#the-td-element)

Status: reviewed (2026-11-01)
> `HTMLTableCellElement`.

#### [4.9.10 The th element](https://html.spec.whatwg.org/#the-th-element)

Status: reviewed (2026-11-01)
> `HTMLTableCellElement`. Fixed: `scope` wasn't limited to its known values.
<!-- definitions: attr-th-scope attr-th-abbr -->

- [x] [Auto](https://html.spec.whatwg.org/#attr-th-scope-auto-state) — The attribute's missing value default and invalid value default are both the Auto state. (In this state the header cell applies to a set of cells selected based on context.)
  > `EnumeratedAttributeTraits<"scope", HTMLTableCellElement>`: the missing and invalid value default, with no keyword.

#### [4.9.11 Attributes common to td and th elements](https://html.spec.whatwg.org/#attributes-common-to-td-and-th-elements)

Status: reviewed (2026-11-01)
> `HTMLTableCellElement`: `colSpan` (1 to 1000) and `rowSpan` (0 to 65534) are clamped, and `cellIndex` is its index among its row's cells.
<!-- definitions: attr-tdth-colspan attr-tdth-rowspan attr-tdth-headers -->

- [x] [HTMLTableCellElement.cellIndex](https://html.spec.whatwg.org/#dom-tdth-cellindex) — The cellIndex IDL attribute must, if the element has a parent tr element, return the index of the cell's element in the parent element's cells collection. If there is no such parent element, then the attribute must return −1.

- [x] [HTMLTableCellElement.scope](https://html.spec.whatwg.org/#dom-th-scope) — The scope IDL attribute must reflect the content attribute of the same name, limited to only known values.

#### [4.9.12 Processing model](https://html.spec.whatwg.org/#table-processing-model)

Status: n/a (the table model is for header cells and accessibility; layout follows CSS tables)
<!-- definitions: table-model concept-slots concept-cell concept-row concept-column concept-row-group concept-column-group table-model-error -->

- [x] [table](https://html.spec.whatwg.org/#concept-table) — A table consists of cells aligned on a two-dimensional grid of slots with coordinates (x, y). The grid is finite, and is either empty or has one or more slots. If the grid has one or more slots, then the x coordinates are always in the range 0 ≤ x < xwidth, and the y coordinates are always in the range 0 ≤ y < yheight. If one or both of xwidth and yheight are zero, then the table is empty (has no slots). Tables correspond to table elements.

##### [4.9.12.1 Forming a table](https://html.spec.whatwg.org/#forming-a-table)

Status: n/a (the table model is for header cells and accessibility; layout follows CSS tables)
<!-- definitions: concept-table-advance -->

- [x] [To determine which elements correspond to which slots in a table associated wit…](https://html.spec.whatwg.org/#forming-a-table) — To determine which elements correspond to which slots in a table associated with a table element, to determine the dimensions of the table (xwidth and yheight), and to determine if there are any table model errors, user agents must use the following algorithm:
  - 1. Let xwidth be 0.
  - 2. Let yheight be 0.
  - 3. Let pending tfoot elements be a list of tfoot elements, initially empty.
  - 4. Let the table be the table represented by the table element. The xwidth and yheight variables give the table's dimensions. The table is initially empty.
  - 5. If the table element has no children elements, then return the table (which will be empty).
  - 6. Associate the first caption element child of the table element with the table. If there are no such children, then it has no associated caption element.
  - 7. Let the current element be the first element child of the table element. If a step in this algorithm ever requires the current element to be advanced to the next child of the table when there is no such next child, then the user agent must jump to the step labeled end, near the end of this algorithm.
  - 8. While the current element is not one of the following elements, advance the current element to the next child of the table: colgroup; thead; tbody; tfoot; tr
  - 9. If the current element is a colgroup, follow these substeps:
    - 9a. Column groups: Process the current element according to the appropriate case below: If the current element has any col element children Follow these steps: If the current element has no col element children
      - 9a.i. Let xstart have the value of xwidth.
      - 9a.ii. Let the current column be the first col element child of the colgroup element.
      - 9a.iii. Columns: If the current column col element has a span attribute, then parse its value using the rules for parsing non-negative integers. If the result of parsing the value is not an error or zero, then let span be that value. Otherwise, if the col element has no span attribute, or if trying to parse the attribute's value resulted in an error or zero, then let span be 1. If span is greater than 10…
      - 9a.iv. Increase xwidth by span.
      - 9a.v. Let the last span columns in the table correspond to the current column col element.
      - 9a.vi. If current column is not the last col element child of the colgroup element, then let the current column be the next col element child of the colgroup element, and return to the step labeled columns.
      - 9a.vii. Let all the last columns in the table from x=xstart to x=xwidth-1 form a new column group, anchored at the slot (xstart, 0), with width xwidth-xstart, corresponding to the colgroup element.
      - 9a.i. If the colgroup element has a span attribute, then parse its value using the rules for parsing non-negative integers. If the result of parsing the value is not an error or zero, then let span be that value. Otherwise, if the colgroup element has no span attribute, or if trying to parse the attribute's value resulted in an error or zero, then let span be 1. If span is greater than 1000, let it be…
      - 9a.ii. Increase xwidth by span.
      - 9a.iii. Let the last span columns in the table form a new column group, anchored at the slot (xwidth-span, 0), with width span, corresponding to the colgroup element.
    - 9b. Advance the current element to the next child of the table.
    - 9c. While the current element is not one of the following elements, advance the current element to the next child of the table: colgroup; thead; tbody; tfoot; tr
    - 9d. If the current element is a colgroup element, jump to the step labeled column groups above.
  - 10. Let ycurrent be 0.
  - 11. Let the list of downward-growing cells be an empty list.
  - 12. Rows: While the current element is not one of the following elements, advance the current element to the next child of the table: thead; tbody; tfoot; tr
  - 13. If the current element is a tr, then run the algorithm for processing rows, advance the current element to the next child of the table, and return to the step labeled rows.
  - 14. Run the algorithm for ending a row group.
  - 15. If the current element is a tfoot, then add that element to the list of pending tfoot elements, advance the current element to the next child of the table, and return to the step labeled rows.
  - 16. The current element is either a thead or a tbody. Run the algorithm for processing row groups.
  - 17. Advance the current element to the next child of the table.
  - 18. Return to the step labeled rows.
  - 19. End: For each tfoot element in the list of pending tfoot elements, in tree order, run the algorithm for processing row groups.
  - 20. If there exists a row or column in the table containing only slots that do not have a cell anchored to them, then this is a table model error.
  - 21. Return the table.

- [x] [algorithm for processing row groups](https://html.spec.whatwg.org/#algorithm-for-processing-row-groups) — The algorithm for processing row groups, which is invoked by the set of steps above for processing thead, tbody, and tfoot elements, is:
  - 1. Let ystart have the value of yheight.
  - 2. For each tr element that is a child of the element being processed, in tree order, run the algorithm for processing rows.
  - 3. If yheight > ystart, then let all the last rows in the table from y=ystart to y=yheight-1 form a new row group, anchored at the slot with coordinate (0, ystart), with height yheight-ystart, corresponding to the element being processed.
  - 4. Run the algorithm for ending a row group.

- [x] [algorithm for ending a row group](https://html.spec.whatwg.org/#algorithm-for-ending-a-row-group) — The algorithm for ending a row group, which is invoked by the set of steps above when starting and ending a block of rows, is:
  - 1. While ycurrent is less than yheight, follow these steps:
    - 1a. Run the algorithm for growing downward-growing cells.
    - 1b. Increase ycurrent by 1.
  - 2. Empty the list of downward-growing cells.

- [x] [algorithm for processing rows](https://html.spec.whatwg.org/#algorithm-for-processing-rows) — The algorithm for processing rows, which is invoked by the set of steps above for processing tr elements, is:
  - 1. If yheight is equal to ycurrent, then increase yheight by 1. (ycurrent is never greater than yheight.)
  - 2. Let xcurrent be 0.
  - 3. Run the algorithm for growing downward-growing cells.
  - 4. If the tr element being processed has no td or th element children, then increase ycurrent by 1, abort this set of steps, and return to the algorithm above.
  - 5. Let current cell be the first td or th element child in the tr element being processed.
  - 6. Cells: While xcurrent is less than xwidth and the slot with coordinate (xcurrent, ycurrent) already has a cell assigned to it, increase xcurrent by 1.
  - 7. If xcurrent is equal to xwidth, increase xwidth by 1. (xcurrent is never greater than xwidth.)
  - 8. If the current cell has a colspan attribute, then parse that attribute's value, and let colspan be the result. If parsing that value failed, or returned zero, or if the attribute is absent, then let colspan be 1, instead. If colspan is greater than 1000, let it be 1000 instead.
  - 9. If the current cell has a rowspan attribute, then parse that attribute's value, and let rowspan be the result. If parsing that value failed or if the attribute is absent, then let rowspan be 1, instead. If rowspan is greater than 65534, let it be 65534 instead.
  - 10. Let cell grows downward be false.
  - 11. If rowspan is zero, then set cell grows downward to true and set rowspan to 1.
  - 12. If xwidth < xcurrent+colspan, then let xwidth be xcurrent+colspan.
  - 13. If yheight < ycurrent+rowspan, then let yheight be ycurrent+rowspan.
  - 14. Let the slots with coordinates (x, y) such that xcurrent ≤ x < xcurrent+colspan and ycurrent ≤ y < ycurrent+rowspan be covered by a new cell c, anchored at (xcurrent, ycurrent), which has width colspan and height rowspan, corresponding to the current cell element. If the current cell element is a th element, let this new cell c be a header cell; otherwise, let it be a data cell. To establish whic…
  - 15. If cell grows downward is true, then add the tuple {c, xcurrent, colspan} to the list of downward-growing cells.
  - 16. Increase xcurrent by colspan.
  - 17. If current cell is the last td or th element child in the tr element being processed, then increase ycurrent by 1, abort this set of steps, and return to the algorithm above.
  - 18. Let current cell be the next td or th element child in the tr element being processed.
  - 19. Return to the step labeled cells.

- [x] [algorithm for growing downward-growing cells](https://html.spec.whatwg.org/#algorithm-for-growing-downward-growing-cells) — When the algorithms above require the user agent to run the algorithm for growing downward-growing cells, the user agent must, for each {cell, cellx, width} tuple in the list of downward-growing cells, if any, extend the cell cell so that it also covers the slots with coordinates (x, ycurrent), where cellx ≤ x < cellx+width.

##### [4.9.12.2 Forming relationships between data cells and header cells](https://html.spec.whatwg.org/#header-and-data-cell-semantics)

Status: n/a (the table model is for header cells and accessibility; layout follows CSS tables)

- [x] [algorithm for assigning header cells](https://html.spec.whatwg.org/#algorithm-for-assigning-header-cells) — Each cell can be assigned zero or more header cells. The algorithm for assigning header cells to a cell principal cell is as follows.
  - 1. Let header list be an empty list of cells.
  - 2. Let (principalx, principaly) be the coordinate of the slot to which the principal cell is anchored.
  - 3. If the principal cell has a headers attribute specified If principal cell does not have a headers attribute specified
    - 3a. Take the value of the principal cell's headers attribute and split it on ASCII whitespace, letting id list be the list of tokens obtained.
    - 3b. For each token in the id list, if the first element in the Document with an ID equal to the token is a cell in the same table, and that cell is not the principal cell, then add that cell to header list.
    - 3a. Let principalwidth be the width of the principal cell.
    - 3b. Let principalheight be the height of the principal cell.
    - 3c. For each value of y from principaly to principaly+principalheight-1, run the internal algorithm for scanning and assigning header cells, with the principal cell, the header list, the initial coordinate (principalx, y), and the increments Δx=−1 and Δy=0.
    - 3d. For each value of x from principalx to principalx+principalwidth-1, run the internal algorithm for scanning and assigning header cells, with the principal cell, the header list, the initial coordinate (x, principaly), and the increments Δx=0 and Δy=−1.
    - 3e. If the principal cell is anchored in a row group, then add all header cells that are row group headers and are anchored in the same row group with an x-coordinate less than or equal to principalx+principalwidth-1 and a y-coordinate less than or equal to principaly+principalheight-1 to header list.
    - 3f. If the principal cell is anchored in a column group, then add all header cells that are column group headers and are anchored in the same column group with an x-coordinate less than or equal to principalx+principalwidth-1 and a y-coordinate less than or equal to principaly+principalheight-1 to header list.
  - 4. Remove all the empty cells from the header list.
  - 5. Remove any duplicates from the header list.
  - 6. Remove principal cell from the header list if it is there.
  - 7. Assign the headers in the header list to the principal cell.

- [x] [internal algorithm for scanning and assigning header cells](https://html.spec.whatwg.org/#internal-algorithm-for-scanning-and-assigning-header-cells) — The internal algorithm for scanning and assigning header cells, given a principal cell, a header list, an initial coordinate (initialx, initialy), and Δx and Δy increments, is as follows:
  - 1. Let x equal initialx.
  - 2. Let y equal initialy.
  - 3. Let opaque headers be an empty list of cells.
  - 4. If principal cell is a header cellLet in header block be true, and let headers from current header block be a list of cells containing just the principal cell.OtherwiseLet in header block be false and let headers from current header block be an empty list of cells.
  - 5. Loop: Increment x by Δx; increment y by Δy.
  - 6. If either x or y are less than 0, then abort this internal algorithm.
  - 7. If there is no cell covering slot (x, y), or if there is more than one cell covering slot (x, y), return to the substep labeled loop.
  - 8. Let current cell be the cell covering slot (x, y).
  - 9. If current cell is a header cell If current cell is a data cell and in header block is trueSet in header block to false. Add all the cells in headers from current header block to the opaque headers list, and empty the headers from current header block list.
    - 9a. Set in header block to true.
    - 9b. Add current cell to headers from current header block.
    - 9c. Let blocked be false.
    - 9d. If Δx is 0 If there are any cells in the opaque headers list anchored with the same x-coordinate as the current cell, and with the same width as current cell, then let blocked be true. If the current cell is not a column header, then let blocked be true. If Δy is 0 If there are any cells in the opaque headers list anchored with the same y-coordinate as the current cell, and with the same height a…
    - 9e. If blocked is false, then add the current cell to the header list.
  - 10. Return to the step labeled loop.

- [x] [column header](https://html.spec.whatwg.org/#column-header) — A header cell anchored at the slot with coordinate (x, y) with width width and height height is said to be a column header if any of the following are true:

- [x] [row header](https://html.spec.whatwg.org/#row-header) — A header cell anchored at the slot with coordinate (x, y) with width width and height height is said to be a row header if any of the following are true:

- [x] [column group header](https://html.spec.whatwg.org/#column-group-header) — A header cell is said to be a column group header if its scope attribute is in the Column Group state.

- [x] [row group header](https://html.spec.whatwg.org/#row-group-header) — A header cell is said to be a row group header if its scope attribute is in the Row Group state.

- [x] [empty cell](https://html.spec.whatwg.org/#empty-cell) — A cell is said to be an empty cell if it contains no elements and its child text content, if any, consists only of ASCII whitespace.

#### [4.9.13 Examples](https://html.spec.whatwg.org/#table-examples)

Status: n/a (examples)
