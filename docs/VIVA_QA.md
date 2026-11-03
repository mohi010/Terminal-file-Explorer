# Viva Voce Questions & Answers: Operating Systems (Storage Management)

This document contains 42 questions and precise answers tailored for university viva examinations covering Operating Systems Unit IV (Storage & File System Management) and this project.

---

### Part 1: File Concepts & Metadata
1. **What is a file in an operating system?**  
   *Answer:* A file is an abstract, named collection of contiguous logical bytes persisted on non-volatile secondary storage.

2. **What is file metadata?**  
   *Answer:* Data about the file, including name, type, size, location, access permissions, owner ID, and timestamps, stored separately from the file data.

3. **What is an inode in Unix/Linux filesystems?**  
   *Answer:* An Inode (index node) is a fixed-size data structure storing all metadata and disk block pointers for a file, excluding its filename.

4. **Where is the filename stored if not in the inode?**  
   *Answer:* Filenames are stored inside directory entry data blocks, which map each human-readable name to its corresponding inode number.

5. **What is the difference between file creation time and modification time?**  
   *Answer:* Creation time (`ctime`) records when the file/inode was initially created or metadata changed; modification time (`mtime`) records when the file content was last written.

6. **Why does the file explorer show directory size as 0 B in listings?**  
   *Answer:* Standard directory entries report the size of the directory table itself (or 0), not the recursive sum of contents, which requires an expensive tree traversal ($O(N)$ system calls).

7. **How does the project calculate recursive directory size (`du`)?**  
   *Answer:* By traversing all descendant nodes via `recursive_directory_iterator`, skipping symlinks, and summing the `file_size` of regular files.

8. **How does text file preview differentiate binary files from plain text?**  
   *Answer:* By inspecting the first 512 bytes for a NUL byte (`\0`) or non-printable control characters.

---

### Part 2: Directory Structures & Navigation
9. **Explain the single-level directory structure.**  
   *Answer:* All files reside in a single directory; filenames must be globally unique across all users, leading to naming collisions.

10. **What is a two-level directory structure?**  
    *Answer:* Each user has an isolated User File Directory (UFD) subordinate to a Master File Directory (MFD), resolving multi-user naming conflicts.

11. **What is a tree-structured directory?**  
    *Answer:* A root directory contains files and subdirectories, forming an arbitrary-depth tree hierarchy with full path isolation.

12. **What is an acyclic-graph directory?**  
    *Answer:* A directory structure allowing shared subdirectories and files via links without cycles.

13. **Why are cycles in directory structures dangerous?**  
    *Answer:* Cyclic links cause infinite recursion during searches, file size calculations, and backup traversals, and complicate reference-counting garbage collection.

14. **How does this project prevent cyclic traversal?**  
    *Answer:* The search and size functions do not follow directory symlinks (`no follow_directory_symlink`).

15. **What is the Current Working Directory (CWD)?**  
    *Answer:* An active path stored in a process's PCB relative to which all relative paths are resolved.

16. **What do `.` and `..` represent in path resolution?**  
    *Answer:* `.` points to the directory itself; `..` points to its parent directory in the hierarchical tree.

17. **What is path canonicalization?**  
    *Answer:* Resolving all symbolic links, relative references (`.`, `..`), and redundant separators into a single unique absolute path.

18. **Why does navigating to a new path clear the forward history?**  
    *Answer:* Like web browsers, branching to a new directory invalidates the linear forward branch.

---

### Part 3: Links & File System Mutation
19. **What is the difference between a hard link and a soft (symbolic) link?**  
    *Answer:* A hard link is another directory entry pointing to the same existing inode; a soft link is an independent file containing the path string of another file.

20. **Can a hard link cross file system boundaries?**  
    *Answer:* No, because inode numbers are only unique within a specific filesystem instance.

21. **Can a symbolic link cross file system boundaries?**  
    *Answer:* Yes, because it stores an arbitrary path string resolved at access time.

22. **What happens to a file when it is deleted using `rm`?**  
    *Answer:* The OS removes the directory entry and decrements the inode's link count; when link count reaches zero and no open handles exist, disk blocks are marked free.

23. **What is a dangling / broken symlink?**  
    *Answer:* A symbolic link whose target path has been deleted or moved.

24. **Why is renaming a file within the same drive an $O(1)$ operation?**  
    *Answer:* It only modifies the directory entry name pointer, requiring zero copying of data blocks.

25. **Why does moving a file across different drives require copying?**  
    *Answer:* Different drives reside on separate storage partitions/controllers, requiring data blocks to be physically read, written, and then deleted from the source.

---

### Part 4: File Allocation Methods
26. **What is Contiguous Allocation?**  
    *Answer:* Each file occupies a contiguous set of physical blocks on disk. Fast sequential access, but causes external fragmentation.

27. **What is Linked Allocation?**  
    *Answer:* Each block contains a pointer to the next block. Eliminates external fragmentation, but slow for random access and vulnerable to pointer corruption.

28. **What is Indexed Allocation?**  
    *Answer:* Each file has an index block containing pointers to all its data blocks. Supports fast direct random access without external fragmentation.

29. **How does Unix FFS/ext combine allocation strategies?**  
    *Answer:* An inode contains direct block pointers, a single indirect pointer, a double indirect pointer, and a triple indirect pointer to support small to massive files.

---

### Part 5: Free Space Management & Disk Storage
30. **What is a Free Space Bit Vector (Bitmap)?**  
    *Answer:* An array of bits where 0 indicates an allocated block and 1 indicates a free block; highly efficient for hardware bit-manipulation instructions.

31. **What is the Linked Free List approach?**  
    *Answer:* Free disk blocks are linked together via pointers embedded inside the free blocks themselves.

32. **What is the Counting method for free space?**  
    *Answer:* Keeps the address of the first free block and the count of contiguous free blocks following it.

33. **What information does `std::filesystem::space` return?**  
    *Answer:* `capacity` (total disk space), `free` (total unallocated space), and `available` (space available to unprivileged users).

---

### Part 6: System Calls & Implementation Architecture
34. **Which Win32 APIs correspond to directory traversal?**  
    *Answer:* `FindFirstFileW`, `FindNextFileW`, and `FindClose`.

35. **Which POSIX system calls correspond to directory traversal?**  
    *Answer:* `opendir`, `readdir`, and `closedir`.

36. **Why does the core library avoid throwing exceptions?**  
    *Answer:* To ensure exception-safe execution across GUI/CLI boundaries and maintain deterministic performance using `Result<T>`.

37. **Why is `SetConsoleOutputCP(CP_UTF8)` needed on Windows?**  
    *Answer:* Windows consoles default to legacy OEM code pages (e.g. CP437); setting UTF-8 ensures Unicode filenames display correctly.

38. **What is Virtual Terminal Processing in the Windows console?**  
    *Answer:* A console mode flag (`ENABLE_VIRTUAL_TERMINAL_PROCESSING`) that enables parsing of standard ANSI color and formatting escape codes.

39. **Why must a directory not be copied into itself?**  
    *Answer:* It would cause infinite recursion, creating nested child copies until disk space or path length limits are exhausted.

40. **How does the project enforce protection on system paths?**  
    *Answer:* `fx::isProtectedPath` canonicalizes target paths and refuses deletion/modification of drive roots, Windows/system directories, and user home.

41. **Why does `weakly_canonical` differ from `canonical`?**  
    *Answer:* `canonical` requires all path components to exist; `weakly_canonical` resolves existing prefixes while leaving non-existent leaf targets intact for creation.

42. **Why does the GUI use a custom `QAbstractTableModel` instead of `QFileSystemModel`?**  
    *Answer:* To preserve the core library (`fxcore`) as the single source of truth for all filesystem logic and operations.

