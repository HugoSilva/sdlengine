#pragma once

// The scene commands the File menu and the keyboard share. They log their own
// failures: a menu item that cannot do what it says has nowhere to return an
// error to.
void SaveScene();
void ReloadScene();
