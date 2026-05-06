# Templates — Interview Q&A

> Eight Q&A pairs. Full answers, common traps, senior follow-ups.

---

**Q1: What is SFINAE and how does `enable_if` implement it?**

**A:** SFINAE (Substitution Failure Is Not An Error) means that when the compiler substitutes template arguments and encounters an invalid type expression, it silently removes that template from the overload candidate set rather than issuing an error. `std::enable_if<B, T>::type` is `T` when `B` is true, and has no member `::type` when `B` is false — causing substitution failure on the false branch. This lets you write mutually exclusive overloads: one enabled for arithmetic types, another for non-arithmetic. If no overload survives substitution, you get a normal "no matching function" error rather than a substitution failure.

**Trap:** "SFINAE applies to the function body." SFINAE only applies during the signature substitution phase of overload resolution — when the compiler is trying to deduce or substitute template arguments into the function's parameter types and return type. Code in the function body is not subject to SFINAE.

**Follow-up:** Why is SFINAE on the return type different from SFINAE on a default template parameter? Functionally similar, but return-type SFINAE can sometimes conflict with explicit specialization. Both approaches work; the C++20 way (concepts) supersedes both.

---

**Q2: What are C++20 concepts and how do they differ from SFINAE?**

**A:** Concepts are named compile-time predicates. You define one with `template<typename T> concept Name = requires { ... }` or by composing existing concepts. At a call site that fails a concept constraint, the error message names the violated concept and the specific requirement that failed — instead of pages of `enable_if` substitution errors. Beyond readability, concepts introduce **subsumption**: in overload resolution, a more constrained template (one whose concept subsumes another) is preferred without requiring `enable_if` tricks. This is a capability SFINAE cannot provide cleanly.

**Trap:** "Concepts are just syntactic sugar for SFINAE." Subsumption is genuinely new. Ordered overload resolution among constrained templates — without explicit SFINAE ranking — is something SFINAE cannot express without significant scaffolding.

**Follow-up:** What is the difference between a `requires` clause and a `requires` expression? A `requires` clause (`template<typename T> requires Condition<T>`) constrains a template. A `requires` expression (`requires(T x) { x.size(); }`) is an expression that evaluates to `bool` at compile time — it tests whether the enclosed operations are well-formed for `T`. Clauses use expressions; they are different syntactic constructs.

---

**Q3: What is two-phase name lookup and why does it matter?**

**A:** Templates are looked up in two phases. Phase 1 happens at the template definition: non-dependent names (those not involving template parameters) must be visible then. Phase 2 happens at instantiation: dependent names (those that depend on `T`) are looked up in the definition context plus ADL in the argument's namespaces. This means you cannot call a function defined after the template if the call is non-dependent — you must declare it before the template. Dependent names defer lookup to instantiation, which is why `this->member()` in a template class makes a base-class member accessible even when it is in a dependent base.

**Trap:** "I can call a function defined anywhere in the file from a template." Only if the call is dependent (through `T`). Non-dependent calls require the function to be declared before the template definition.

**Follow-up:** Why do you sometimes need `this->foo()` inside a template class method? In a class template `Derived : Base<T>`, the name `foo` without `this->` is non-dependent — the compiler looks it up in phase 1, before it knows what `Base<T>` contains. Adding `this->` makes `foo` dependent, deferring lookup to phase 2 where the base class instantiation is visible.

---

**Q4: What is a variadic template and when would you use fold expressions vs recursion?**

**A:** A variadic template accepts any number of type (`typename... Ts`) or non-type (`int... Ns`) template parameters. You expand the pack with `Ts...` or `args...`. Recursive unpacking works by pattern-matching the pack into a head and tail, handling the head, and recursing on the tail with a base case for zero elements — generating N+1 instantiations. Fold expressions (`(args + ...)`) apply a binary operator across the pack in a single expression — no recursion, no base case, far less template instantiation. Use folds for arithmetic, logical, and comma-operator combinations. Use recursion when the per-element logic requires stateful iteration that fold expressions cannot express.

**Trap:** Forgetting the base case in recursive variadic templates. Without it, the recursion continues until `sizeof...(rest) == 0`, hits the template instantiation with an empty pack, and fails — compile error or infinite recursion error.

**Follow-up:** What is the difference between a left fold and a right fold? Left fold: `(... op pack)` evaluates as `((a op b) op c)` — left-associative. Right fold: `(pack op ...)` evaluates as `(a op (b op c))` — right-associative. For addition and multiplication, both are equivalent. For subtraction and function application (like `push_back` with `,` operator), associativity matters.

---

**Q5: What is policy-based design?**

**A:** Policy-based design parameterizes a class on policy classes that inject specific behaviors at compile time via template parameters. The host class delegates operations to the policy's static functions or types. Multiple independent policies can be composed as multiple template parameters. This is the compile-time equivalent of the Strategy pattern: `Sorter<AscendingPolicy>` and `Sorter<DescendingPolicy>` are different types, each with its comparison fully inlined. There is no virtual dispatch, no function pointer, no runtime overhead. Alexandrescu documented this in "Modern C++ Design" (2001) and it remains the idiom for high-performance, highly configurable systems.

**Trap:** Confusing policy with runtime strategy. They are conceptually the same pattern, but policy is resolved at compile time (different binary generated per policy combination) while strategy is a runtime decision (one binary, virtual call to the current strategy). Use policy for performance-critical, compile-time-configurable components. Use strategy for runtime-switchable behavior.

**Follow-up:** What is the cost of policy-based design? Longer compile times (each policy combination is a separate instantiation), larger binaries if many combinations are used, and harder-to-read template error messages when a policy does not satisfy its expected interface. Writing a concept for the policy interface mitigates the last issue.

---

**Q6: What are expression templates and why are they useful?**

**A:** Expression templates encode an arithmetic expression as a compile-time tree of proxy types rather than evaluating it immediately. `operator+` on two vectors returns a `VecAdd<Vec, Vec>` proxy — not a `Vec`. Assignment from the proxy to a `Vec` evaluates it element-by-element in a single loop: `a[i] = b[i] + c[i]` with no temporaries. Chained operations (`a = b + c + d`) produce `VecAdd<VecAdd<Vec,Vec>, Vec>` — still no temporaries. Assignment evaluates the full tree in one pass. Expression templates eliminate all intermediate allocations from multi-operand arithmetic — the technique used by Eigen, Blaze, and xtensor for matrix/vector operations.

**Trap:** "Move semantics can do the same thing." Move semantics eliminate copies, but each intermediate result still exists — `b + c` still allocates storage for the intermediate `Vec`, which is then moved into the next operand's expression. Expression templates eliminate the intermediate entirely — no allocation, no move, just index arithmetic in the assignment loop.

**Follow-up:** What is the danger of expression templates? The proxy stores references to its operands. If you store the expression in a variable and the operands go out of scope (e.g., the operands are temporaries), the proxy holds dangling references. Expression templates must be consumed immediately. `auto result = a + b;` stores the proxy — the operands must outlive `result`.

---

**Q7: What is the difference between full and partial template specialization?**

**A:** Full specialization provides a complete, specific implementation for a particular set of template arguments: `template<> class Stack<bool>` is a fully specialized class for `Stack` with `T = bool`. Partial specialization provides an implementation for a pattern of arguments, leaving some still generic: `template<typename T> class Stack<T*>` matches `Stack<any pointer type>`. Only class templates and variable templates can be partially specialized. Function templates cannot — the standard explicitly disallows it. For function templates, achieve similar effects through overloading: `template<typename T> void f(T*)` is an overload for pointer types, not a specialization.

**Trap:** "I can partially specialize function templates." Not allowed. GCC accepts it in some cases as an extension, but it is non-portable and formally disallowed by the standard. Write overloads instead.

**Follow-up:** What is explicit instantiation and when would you use it? Explicit instantiation (`template class Sorter<int>;` in a `.cpp`) forces the compiler to generate the template instance in exactly one translation unit. All other translation units that use `Sorter<int>` link to this one copy rather than generating their own. This reduces binary size and link time in large codebases where a template is used in many translation units. Pair with an explicit instantiation declaration (`extern template class Sorter<int>;` in the header) to suppress per-TU instantiation.

---

**Q8: What is a type list and what can you do with one?**

**A:** A type list (`TypeList<int, double, char>`) is a variadic template that carries a sequence of types as compile-time data — a list of types, zero runtime bytes. Operations: `Head<L>` (first type), `Tail<L>` (rest as a new TypeList), `Size<L>` (count), `At<N, L>` (N-th type), `Contains<T, L>` (membership test), `Append<L, T>` (add to end), `Filter<L, Pred>` (keep types satisfying a predicate). These are all computed at compile time via template specialization and recursive pattern matching. Used in tuple implementations, type-safe heterogeneous containers, and compile-time dispatch tables.

**Trap:** Thinking type lists have runtime overhead. They are entirely compile-time constructs. `TypeList<int, double>` occupies zero bytes at runtime — the types are only metadata for the compiler.

**Follow-up:** How would you implement `At<N, TypeList<Ts...>>`? Recursive specialization: base case `At<0, TypeList<T, Rest...>>` has `type = T` (the head). Recursive case `At<N, TypeList<T, Rest...>>` inherits from `At<N-1, TypeList<Rest...>>`, stripping the head and decrementing N. When N reaches 0, the base case matches. For out-of-bounds, a `static_assert` in the base case on empty TypeList catches it.
