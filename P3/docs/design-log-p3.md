# Project 3 Design Log

**Author:** Julian L. Mitchell 
**P2 code used:** My own code
**Tools used:** ChatGPT helped with some of the brainstorming and refining the code

## 1. Tool registry
I implemented separate chaining using a vector of buckets, where each bucket is a vector of `std::unique_ptr<Tool>`. Collisions just append to the vector in that bucket, and `find()` linear searches through that specific chain.

The table starts at 16 buckets. Before inserting, it checks if the load factor would exceed 0.75; if so, it doubles the bucket array and rehashes everything. FNV-1a is used for the string hash since it guarantees every character influences the final hash.

## 2. Tool ownership
Using `std::unique_ptr` ensures the registry takes full ownership and cleans up memory automatically. When rehashing, moving the unique pointers moves ownership without relocating the actual `Tool` objects on the heap, which keeps the pointers returned by `find()` completely stable.

## 3. Workspace paths
`Workspace::resolve()` checks the input string for a valid path. It catches empty paths, embedded NUL bytes, and absolute paths first. Then it calls `lexically_normal()` and checks if the first component is `..` to block any directory escapes.

## 4. Calculator evaluation
The evaluator uses a two-stack algorithm (one for numbers, one for operators). Precedence checks ensure that multiplication and division happen before addition and subtraction, and the `>=` precedence rule preserves left-to-right association. For `2 * (3 + 4) - 5`, the parentheses evaluate first, then multiplication runs, and subtraction finishes last, resulting in 9.

## 5. Test coverage
I wrote 15 test cases covering registry operations, 10,000-item scaling, path validation rules, and calculator precedence scenarios, all of which passed.

## 6. One thing I would change for a larger program
I'd probably wrap the hash table logic into a clean template class instead of baking it directly into `ToolRegistry`.