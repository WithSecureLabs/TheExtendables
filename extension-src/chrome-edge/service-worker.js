chrome.runtime.onStartup.addListener(function() {
  chrome.runtime.connectNative('com.withsecure.theextendables');
})