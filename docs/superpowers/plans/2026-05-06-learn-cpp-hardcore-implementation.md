# Hardcore C++ Fundamentals (Compiler Track) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rewrite `projects/learn-cpp-beginner/` into a self-contained hardcore C++ fundamentals “book + handbook” (hybrid core+dives) optimized for building a compiler.

**Architecture:** Keep the existing folder, but introduce `core/` (long-form booklets) and `dives/` (indexed deep dives). Update `README.md` into a TOC + practice guide. Keep existing beginner chapters as a `legacy/` snapshot (non-destructive migration).

**Tech Stack:** Markdown, Mermaid (where helpful), GCC 11 compatibility constraints reflected in docs.

---

## File structure (created/modified)

**Create:**
- `projects/learn-cpp-beginner/core/01-object-model-and-lifetime.md`
- `projects/learn-cpp-beginner/dives/language/010-value-categories-and-reference-collapsing.md`
- `projects/learn-cpp-beginner/dives/language/020-temporaries-and-lifetime-extension.md`
- `projects/learn-cpp-beginner/dives/language/030-copy-move-and-guaranteed-elision.md`
- `projects/learn-cpp-beginner/dives/language/040-initialization-and-narrowing.md`
- `projects/learn-cpp-beginner/dives/language/050-ub-taxonomy-and-how-it-bites.md`
- `projects/learn-cpp-beginner/dives/stdlib/110-vector-growth-invalidation-complexity.md`
- `projects/learn-cpp-beginner/dives/memory/210-raii-scopeguard-and-exception-unwinding.md`
- `projects/learn-cpp-beginner/dives/memory/220-unique_ptr-shared_ptr-weak_ptr-ownership-graphs.md`
- `projects/learn-cpp-beginner/legacy/README.md`

**Modify:**
- `projects/learn-cpp-beginner/README.md`

**Move (rename, preserving content):**
- `projects/learn-cpp-beginner/01-how-cpp-programs-work.md` → `projects/learn-cpp-beginner/legacy/01-how-cpp-programs-work.md`
- `projects/learn-cpp-beginner/02-the-stl-and-standard-library.md` → `projects/learn-cpp-beginner/legacy/02-the-stl-and-standard-library.md`
- `projects/learn-cpp-beginner/03-memory-raii-and-smart-pointers.md` → `projects/learn-cpp-beginner/legacy/03-memory-raii-and-smart-pointers.md`
- `projects/learn-cpp-beginner/04-classes-oop-and-polymorphism.md` → `projects/learn-cpp-beginner/legacy/04-classes-oop-and-polymorphism.md`
- `projects/learn-cpp-beginner/05-templates-and-concepts-intro.md` → `projects/learn-cpp-beginner/legacy/05-templates-and-concepts-intro.md`
- `projects/learn-cpp-beginner/06-patterns-concurrency-and-modern-types.md` → `projects/learn-cpp-beginner/legacy/06-patterns-concurrency-and-modern-types.md`
- `projects/learn-cpp-beginner/07-build-quality-toolchain-basics.md` → `projects/learn-cpp-beginner/legacy/07-build-quality-toolchain-basics.md`
- `projects/learn-cpp-beginner/08-ros2-vocabulary-and-architecture.md` → `projects/learn-cpp-beginner/legacy/08-ros2-vocabulary-and-architecture.md`
- `projects/learn-cpp-beginner/09-cpp11-through-cpp20-tour.md` → `projects/learn-cpp-beginner/legacy/09-cpp11-through-cpp20-tour.md`

Rationale: this keeps history, avoids deleting anything, and makes the new “hardcore” track unambiguous.

---

### Task 1: Restructure `learn-cpp-beginner/` into `core/`, `dives/`, `legacy/`

**Files:**
- Create: `projects/learn-cpp-beginner/core/.gitkeep` (temporary, if needed)
- Create: `projects/learn-cpp-beginner/dives/{language,stdlib,templates,memory,concurrency,toolchain,perf}/.gitkeep` (temporary, if needed)
- Create: `projects/learn-cpp-beginner/legacy/README.md`
- Move: all existing `projects/learn-cpp-beginner/*.md` chapters into `projects/learn-cpp-beginner/legacy/`

- [ ] **Step 1: Create directories**

Run:
```bash
mkdir -p projects/learn-cpp-beginner/core \
         projects/learn-cpp-beginner/dives/language \
         projects/learn-cpp-beginner/dives/stdlib \
         projects/learn-cpp-beginner/dives/templates \
         projects/learn-cpp-beginner/dives/memory \
         projects/learn-cpp-beginner/dives/concurrency \
         projects/learn-cpp-beginner/dives/toolchain \
         projects/learn-cpp-beginner/dives/perf \
         projects/learn-cpp-beginner/legacy
```

- [ ] **Step 2: Move existing chapters into `legacy/`**

Run:
```bash
git mv projects/learn-cpp-beginner/0{1,2,3,4,5,6,7,8,9}-*.md projects/learn-cpp-beginner/legacy/
```

- [ ] **Step 3: Write `legacy/README.md`**

Create `projects/learn-cpp-beginner/legacy/README.md`:
```md
# Legacy Beginner Notes (Archived)

These files are the original “beginner track” notes. They’re kept for reference, but the active curriculum is now the **Hardcore C++ Fundamentals (Compiler Track)** in the parent directory’s `README.md`, `core/`, and `dives/`.
```

- [ ] **Step 4: Commit**

Run:
```bash
git add projects/learn-cpp-beginner/legacy projects/learn-cpp-beginner/core projects/learn-cpp-beginner/dives
git commit -m "$(cat <<'EOF'
docs: archive beginner track as legacy
EOF
)"
```

---

### Task 2: Rewrite `projects/learn-cpp-beginner/README.md` as the new TOC + practice guide

**Files:**
- Modify: `projects/learn-cpp-beginner/README.md`

- [ ] **Step 1: Replace README with hardcore TOC**

Edit `projects/learn-cpp-beginner/README.md` to include:
- one-paragraph positioning (“compiler track”)
- toolchain note (GCC 11: no `std::format`, no `std::expected`)
- “How to use this” section:
  - core order
  - deep-dive lookup index
  - drills vs capstones
- TOC sections:
  - Core (links to `core/01...` etc; placeholders for future core booklets)
  - Deep dives (grouped by area; include the initial set created in Task 3/4)
  - Legacy link (`legacy/README.md`)

- [ ] **Step 2: Commit**

Run:
```bash
git add projects/learn-cpp-beginner/README.md
git commit -m "$(cat <<'EOF'
docs: rebrand learn-cpp as hardcore compiler track
EOF
)"
```

---

### Task 3: Author Core Booklet 01 — Object model & lifetime (compiler-shaped)

**Files:**
- Create: `projects/learn-cpp-beginner/core/01-object-model-and-lifetime.md`

- [ ] **Step 1: Write Core 01 content**

Create `projects/learn-cpp-beginner/core/01-object-model-and-lifetime.md` with sections:
- Why you care (compiler-shaped): AST nodes, token strings, diagnostics lifetimes
- Mental model: storage duration + object lifetime + ownership graph
- Rules & invariants:
  - destructors and scope exit (RAII)
  - references vs pointers vs values
  - temporaries, lifetime extension (preview; deep dive link)
  - copy/move and when “move” is a lie
  - guaranteed copy elision (C++17+) vs “might happen”
- Failure modes:
  - dangling references (esp. `string_view`)
  - use-after-move patterns
  - double-free via accidental copying
  - exception path leaks when not using RAII
- Idioms:
  - value types for tokens/spans
  - arena allocation concept (preview)
  - “owning vs non-owning” API boundaries
- Standard library contracts used here:
  - `vector` growth/invalidation overview (link deep dive)
  - `string` vs `string_view` contract boundaries (preview)
- Drills (5–10)
- Capstone (1): design an AST node ownership model (choose: owning `unique_ptr` tree vs arena + indices)

- [ ] **Step 2: Commit**

Run:
```bash
git add projects/learn-cpp-beginner/core/01-object-model-and-lifetime.md
git commit -m "$(cat <<'EOF'
docs: add core 01 on object model and lifetime
EOF
)"
```

---

### Task 4: Seed the first “deep dives” that support Core 01

**Files (create):**
- `projects/learn-cpp-beginner/dives/language/010-value-categories-and-reference-collapsing.md`
- `projects/learn-cpp-beginner/dives/language/020-temporaries-and-lifetime-extension.md`
- `projects/learn-cpp-beginner/dives/language/030-copy-move-and-guaranteed-elision.md`
- `projects/learn-cpp-beginner/dives/language/040-initialization-and-narrowing.md`
- `projects/learn-cpp-beginner/dives/language/050-ub-taxonomy-and-how-it-bites.md`
- `projects/learn-cpp-beginner/dives/stdlib/110-vector-growth-invalidation-complexity.md`
- `projects/learn-cpp-beginner/dives/memory/210-raii-scopeguard-and-exception-unwinding.md`
- `projects/learn-cpp-beginner/dives/memory/220-unique_ptr-shared_ptr-weak_ptr-ownership-graphs.md`

- [ ] **Step 1: Write each deep dive using the short-form template**

Each file must include:
- rules/guarantees
- pitfalls
- minimal example(s)
- 5–15 drills
- “Used in compilers as…” mapping bullets

- [ ] **Step 2: Update README deep-dive index**

Add links to these dives under the relevant area headings.

- [ ] **Step 3: Commit**

Run:
```bash
git add projects/learn-cpp-beginner/dives projects/learn-cpp-beginner/README.md
git commit -m "$(cat <<'EOF'
docs: add initial deep dives supporting core 01
EOF
)"
```

---

### Task 5: Self-check (editorial consistency)

**Files:**
- Modify as needed: `projects/learn-cpp-beginner/README.md`, any new booklet

- [ ] **Step 1: Link check**

Run:
```bash
python3 - <<'PY'
import os, re, sys
root="projects/learn-cpp-beginner"
missing=[]
for dirpath, _, filenames in os.walk(root):
    for fn in filenames:
        if not fn.endswith(".md"): continue
        p=os.path.join(dirpath, fn)
        txt=open(p,"r",encoding="utf-8").read()
        for m in re.finditer(r"\[[^\\]]*\\]\\(([^)]+)\\)", txt):
            link=m.group(1)
            if link.startswith("http"): continue
            # strip anchors
            link=link.split("#",1)[0]
            if not link: continue
            target=os.path.normpath(os.path.join(dirpath, link))
            if not os.path.exists(target):
                missing.append((p, link, target))
if missing:
    print("Missing links:")
    for p, link, target in missing:
        print(f"- {p}: {link} -> {target}")
    sys.exit(1)
print("OK: no missing relative links")
PY
```

- [ ] **Step 2: Commit any fixes (if needed)**

Run:
```bash
git add projects/learn-cpp-beginner
git commit -m "$(cat <<'EOF'
docs: fix hardcore track link consistency
EOF
)" || true
```

