

#ifndef _FURIOUS_COMPILER_TRANSFORMS_H_
#define _FURIOUS_COMPILER_TRANSFORMS_H_


struct Foreach;
struct FccContext;
struct FccMatch;
struct FccOperator;

/**
 * @brief  Merges two Foreach operators into a new one. The new Foreach operator
 * executes the functions of the two original foreach over the children table.
 *
 * @param context  The Fcc compilation context
 * @param foreach1 The first Foreach to merge
 * @param foreach2 The secont Foreach to merge
 *
 * @return A newly created Foreach resulting of merging the two input Foreach.
 */
Foreach* 
merge_foreach(FccContext* context,
              const Foreach* foreach1, 
              const Foreach* foreach2);





#endif /* ifndef _FURIOUS_COMPILER_TRANSFORMS_H_ */
