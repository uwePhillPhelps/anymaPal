/*
  AnymaPal - a friend for MIDI recording with Aodyo Anyma Phi
*/

#include "../JuceLibraryCode/JuceHeader.h"

Component* createMainContentComponent();

class AnymaPal  : public JUCEApplication
{
private:
    class MainWindow    : public DocumentWindow
    {
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)

    public:
        MainWindow (String name)
          : DocumentWindow (
                name, Colours::lightgrey, DocumentWindow::allButtons )
        {
            setUsingNativeTitleBar (true);
            setContentOwned (createMainContentComponent(), true);
            setResizable (false, false);

            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }
    }; // class MainWindow
    ScopedPointer<MainWindow> mainWindow;

public:
    AnymaPal() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    void initialise (const String& commandLine) override
    {
        mainWindow = new MainWindow (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
    }
}; // class AnymaPal


// macro to generate main()
START_JUCE_APPLICATION (AnymaPal)
