# Intow
Intow is a small console application that is able to send keys to a specified destination window. It uses a special technique that allows it to send input to application windows that normally would block virtual inputs. Intow stands for "input to window".

# Features
- Sending arbitrary many series of keys, so called inputs, in a loop
- Sending to multiple windows at the same time

# Console interface
Intow is fully controlled by interacting with the console. The commands (capitalization is irrelevant):
- commands: Lists all available commands with a brief description (just like below).
- listWindows: Fetches a list of all top-level open windows and prints it.
- listWindows2: Fetches a list of open windows with mediocre filtering and prints it.
- listWindows3: Fetches a list of all open windows and prints it.
- listInputs: Lists all current inputs.
- focus windowId: Sets the window with the corresponding window id as the foreground window.
- new
- delete
