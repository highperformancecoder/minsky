

### Step-by-Step Instructions to Run `ravel-3.10.0.exe` in Wine

#### 1. Install Wine
Install the latest stable Wine (10.0 or later) from WineHQ’s official repository for best compatibility with Electron apps.[](https://ubuntuhandbook.org/index.php/2022/04/wine-ubuntu-2204-windows-apps/)[](https://www.omgubuntu.co.uk/2023/01/install-wine-on-ubuntu)

```bash
# Enable 32-bit architecture (required for most Windows apps)
sudo dpkg --add-architecture i386
sudo apt update

# Create directory for WineHQ key
sudo mkdir -pm755 /etc/apt/keyrings

# Download and install WineHQ key
sudo wget -O /etc/apt/keyrings/winehq-archive.key https://dl.winehq.org/wine-builds/winehq.key

# Add WineHQ repository for Ubuntu 24.04 (replace 'noble' with 'jammy' for 22.04 or 'focal' for 20.04)
sudo wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/ubuntu/dists/noble/winehq-noble.sources

# Update package list and install Wine stable
sudo apt update
sudo apt install --install-recommends winehq-stable
```

- **Verify Installation**:
  ```bash
  wine --version
  ```
  Expect output like `wine-10.0` or later.

#### 2. Configure Wine
Set up a Wineprefix (virtual Windows environment) for Ravel to avoid conflicts with other apps.[](https://www.wikihow.com/Use-Wine-on-Linux)[](https://linuxhint.com/wine_ubuntu_install_configure/)

```bash
# Create a new Wineprefix
WINEPREFIX=~/.wine_ravel winecfg
```

- In the Wine configuration dialog:
  - Set **Windows Version** to **Windows 10** (Electron apps work best with this).[](https://askubuntu.com/questions/1057339/how-to-configure-wine-to-use-windows-8-or-10)
  - Install **Mono** and **Gecko** when prompted (required for .NET and HTML rendering).
  - In the **Graphics** tab, enable **AllowPixelShader** for better rendering.
  - Close the dialog when done.

- **Install Winetricks** for dependency management:
  ```bash
  sudo apt install winetricks
  ```

- Install common dependencies for Electron/C++ apps:
  ```bash
  WINEPREFIX=~/.wine_ravel winetricks corefonts vcrun2019 msvcrt
  ```
  If Ravel uses Python scripting:
  ```bash
  WINEPREFIX=~/.wine_ravel winetricks python3
  ```

#### 3. Run `ravel-3.10.0.exe`
Navigate to the directory containing `ravel-3.10.0.exe` (assumed to be `~/Downloads`) and run it with Wine.[](https://www.wikihow.com/Use-Wine-on-Linux)[](https://www.zdnet.com/article/how-to-easily-run-windows-apps-on-linux-with-wine/)

```bash
cd ~/Downloads
WINEPREFIX=~/.wine_ravel wine ravel-3.10.0.exe
```

- **Alternative (GUI)**:
  - Right-click `ravel-3.10.0.exe` in the file manager.
  - Select **Open With Wine Windows Program Loader**.

- **Expected Behavior**:
  - If compatible, Ravel’s GUI should launch, displaying its visual editor interface.
  - The app installs to `~/.wine_ravel/drive_c/Program Files` (or `Program Files (x86)` for 32-bit).

#### 4. Post-Installation
- **Launch Ravel**:
  After installation, find the executable in `~/.wine_ravel/drive_c/Program Files` (or `Program Files (x86)`). Run it:
  ```bash
  WINEPREFIX=~/.wine_ravel wine ~/.wine_ravel/drive_c/users/$USER/AppData/Local/Programs/ravel/ravel.exe
  ```
  Adjust the path based on the actual installation directory.

- **Create a Shortcut (untested)**:
  To launch Ravel easily, create a desktop entry:
  ```bash
  nano ~/.local/share/applications/ravel.desktop
  ```
  Add:
  ```ini
  [Desktop Entry]
  Name=Ravel
  Exec=env WINEPREFIX=/home/$USER/.wine_ravel wine /home/$USER/.wine_ravel/drive_c/Program\ Files/Ravel/ravel.exe
  Type=Application
  Icon=/path/to/ravel-icon.png
  Terminal=false
  ```
  Save and make executable:
  ```bash
  chmod +x ~/.local/share/applications/ravel.desktop
  ```

