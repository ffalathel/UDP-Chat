```markdown
# UDP Chat Application (CLI + GUI)

This project implements a peer-to-peer chat system using UDP sockets and a custom delimited string protocol. It supports both:

- ✅ A terminal-based **CLI version**
- ✅ A graphical **GTK-based GUI version**

The two versions are fully interoperable, allowing CLI-to-GUI and GUI-to-GUI communication.

---

## ✨ Features

- UDP transport for message exchange
- Cross-platform compatible (tested on macOS with M1)
- Custom protocol includes:
  - Sender's username and hostname
  - Chat start timestamp
  - Total bytes sent
- Message viewer and input for GUI
- Byte mismatch warning for integrity checks
- Clean threading and shutdown handling in both versions

---

## 🧰 Requirements

- GCC or Clang
- GTK+ 3 (for GUI version)
- `pkg-config` (for compiling GTK GUI)
- Unix-like system (macOS/Linux)

To install dependencies on macOS:

```bash
brew install gtk+3 pkg-config
```

---

## 📁 Project Structure

```
udp_chat_project/
├── Makefile
├── README.md
├── cli/
│   └── main_cli.c
├── gui/
│   └── main_gui.c
├── common/
│   ├── protocol.h / protocol.c
│   ├── transport.h / transport.c
│   └── session.h
```

---

## 🛠️ Build Instructions

To build everything:

```bash
make
```

To build only the CLI version:

```bash
make cli
```

To build only the GUI version:

```bash
make gui
```

To clean all builds:

```bash
make clean
```

---

## 🚀 How to Run

### 🧪 Test CLI-to-CLI

#### Terminal 1:
```bash
./chat_cli --username alice --port 9000 --peer-ip 127.0.0.1 --peer-port 9001
```

#### Terminal 2:
```bash
./chat_cli --username bob --port 9001 --peer-ip 127.0.0.1 --peer-port 9000
```

---

### 🧪 Test GUI-to-GUI

#### Terminal 1:
```bash
./chat_gui alice 9000 127.0.0.1 9001 $(hostname)
```

#### Terminal 2:
```bash
./chat_gui bob 9001 127.0.0.1 9000 $(hostname)
```

> You must run GUI versions from a real terminal like **Terminal.app** or **iTerm2** (not VS Code's terminal).

---

### 🧪 Mix GUI and CLI

Yes! You can run one user in the CLI version and the other in the GUI version — they use the same protocol.

---


## ✅ Submission Checklist

- [x] Fully working CLI
- [x] Fully working GUI
- [x] Clean shutdown & no socket spam
- [x] Compatible across both versions
- [x] Protocol matches all spec requirements
- [x] Includes Makefile and README

---

💬 Built by Fahada Alathel  
Tested on macOS (Apple Silicon)

Happy Chatting!
```
