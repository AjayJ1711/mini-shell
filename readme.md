# Mini Shell in C

A simple UNIX-like shell built in C that supports basic command execution, built-in `cd`, I/O redirection, and command chaining using pipes (`|`).

---

## Features

- ✅ Built-in `cd` command
- ✅ Execute external commands (e.g., `ls`, `cat`, `echo`, etc.)
- ✅ Output redirection (`>`, `>>`)
- ✅ Input redirection (`<`)
- ✅ Command chaining with pipes (`|`)
- ✅ Error handling with informative messages

---

## How to Compile & Run

```bash
gcc shell.c -o mysh
./mysh
```

---

# Example usage

mysh> ls -l
mysh> cat input.txt > output.txt
mysh> sort < unsorted.txt | uniq | tee result.txt
mysh> cd ..

# Limitations / Future Enhancements

❌ Background execution (&) not supported yet

❌ No support for environment variable expansion

❌ No signal handling (e.g., Ctrl+C behavior)

❌ No job control (fg, bg, jobs)

❌ No command history or autocomplete


