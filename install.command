#!/bin/bash
# Synthe · AFTERHOURS — installs the plugin system-wide (asks for your password)
cd "$(dirname "$0")"
echo "Installing Synthe..."
sudo mkdir -p /Library/Audio/Plug-Ins/VST3 /Library/Audio/Plug-Ins/Components
# removes the old Synthe (also old copies in your user folder)
sudo rm -rf /Library/Audio/Plug-Ins/VST3/Synthe.vst3 /Library/Audio/Plug-Ins/Components/Synthe.component
rm -rf ~/Library/Audio/Plug-Ins/VST3/Synthe.vst3 ~/Library/Audio/Plug-Ins/Components/Synthe.component 2>/dev/null
sudo cp -R Synthe.vst3 /Library/Audio/Plug-Ins/VST3/
sudo cp -R Synthe.component /Library/Audio/Plug-Ins/Components/
sudo xattr -cr /Library/Audio/Plug-Ins/VST3/Synthe.vst3 /Library/Audio/Plug-Ins/Components/Synthe.component
killall -9 AudioComponentRegistrar 2>/dev/null
echo "Done. Restart Ableton and rescan plug-ins. Synthe is in the AFTERHOURS folder."
