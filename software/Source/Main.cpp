#include "MainComponent.h"
#include <JuceHeader.h>

class OSSLumatoneEditorApplication : public juce::JUCEApplication {
public:
  const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
  const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
  bool moreThanOneInstanceAllowed() override { return false; }

  void initialise(const juce::String &) override {
    mainWindow = std::make_unique<MainWindow>(getApplicationName());
  }

  void shutdown() override { mainWindow = nullptr; }

  void systemRequestedQuit() override { quit(); }

private:
  class MainWindow : public juce::DocumentWindow {
  public:
    explicit MainWindow(const juce::String &name)
        : DocumentWindow(name, juce::Colour(0xff1e1f24), DocumentWindow::allButtons) {
      setUsingNativeTitleBar(true);
      setContentOwned(new MainComponent(), true);
      setResizable(true, true);
      setResizeLimits(800, 400, 4000, 2000);
      centreWithSize(getWidth(), getHeight());
      setVisible(true);
    }

    void closeButtonPressed() override {
      juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
  };

  std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(OSSLumatoneEditorApplication)
