# ArchShell

**ArchShell** is a lightweight, custom shell built in C, designed to work on Arch Linux or any Linux-based system. It supports basic shell functionality, built-in commands, piping, file redirection, and now **Python scripting** for dynamic extensions.

---

## Features

1. **Custom Prompt**:
   - Displays the current working directory.

2. **Built-in Commands**:
   - `cd [dir]`: Change the current directory.
   - `exit`: Exit the shell.
   - `history`: Display command history.
   - `help`: Display a help menu.

3. **External Commands**:
   - Supports execution of standard Linux commands like `ls`, `pwd`, etc.

4. **Piping (`|`)**:
   - Chain multiple commands together, e.g., `ls | grep shell`.

5. **Redirection**:
   - Output redirection: `command > file`.
   - Input redirection: `command < file`.

6. **Command History**:
   - Keeps track of up to 100 commands during the session.

7. **Python Scripting**:
   - Execute Python scripts directly from the shell by providing the script's filename. The Python interpreter is embedded into the shell.

8. **Signal Handling**:
   - Prevents accidental termination via `Ctrl+C`.

---

## Requirements

To compile the shell, ensure you have `gcc` and Python development libraries installed. On Arch Linux, install them with:

```bash
sudo pacman -S gcc python python-devel


---

## Compilation

To compile the shell, ensure you have `gcc` installed. Run the following command:

```bash
gcc archshell_advanced.c -o archshell
