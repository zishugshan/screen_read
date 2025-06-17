# Screen Read App

This application helps you **read and copy text from any part of the screen**, even when normal copy-paste is not possible (e.g., images or non-selectable UIs).

## Features

- Select an area on the screen
- Capture that as an image
- Extract and copy text from the captured area using OCR

## How to Build

### Option 1: Compile Locally (Linux/macOS)

```bash
gcc -o screen_read screen_read.c api_endpoints.c functions.c
./screen_read

