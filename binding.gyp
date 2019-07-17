{
  'targets': [{
    'target_name': 'aira-uweb',
    'cflags': [
      '-Wall',
      '-Wparentheses',
      '-Winline',
      '-Wbad-function-cast',
      '-Wdisabled-optimization'
    ],
    
    'conditions': [
      ['OS == "mac"', {
        'include_dirs': [
          'System/Library/Frameworks/CoreFoundation.Framework/Headers',
          'System/Library/Frameworks/Carbon.Framework/Headers',
          'System/Library/Frameworks/ApplicationServices.framework/Headers',
          'System/Library/Frameworks/OpenGL.framework/Headers',
        ],
        'link_settings': {
          'libraries': [
            '-framework Carbon',
            '-framework CoreFoundation',
            '-framework ApplicationServices',
            '-framework OpenGL'
          ]
        }
      }],
      
      ['OS == "linux"', {
        'link_settings': {
          'libraries': [
            '-lpng',
            '-lz',
            '-lX11',
            '-lXtst'
          ]
        },
        
        'sources': [
          'src/xdisplay.c'
        ]
      }],

      ["OS=='win'", {
        'defines': ['IS_WINDOWS','LIBUS_USE_LIBUV','LIBUS_NO_SSL=0']
      }]
    ],

    'include_dirs':['uWebSockets/uSockets/src','uWebSockets/src','src'],
    'sources': [
    'uWebSockets/uSockets/src/context.c',
    'uWebSockets/uSockets/src/loop.c',
    'uWebSockets/uSockets/src/socket.c',
     'uWebSockets/uSockets/src/eventing/epoll.c',
     'uWebSockets/uSockets/src/eventing/libuv.c',
    'uWebSockets/uSockets/src/ssl.c',
    'src/addon.cpp'
    ],
    'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': [ '-std:c++17', ],
          },
        }

  }]
}