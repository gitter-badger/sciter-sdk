//
//  window.mm
//  sciter
//
//  Created by andrew on 2014-06-30.
//  Copyright (c) 2014 andrew fedoniouk. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include "window.h"

NSView*   get_nsview(window* w)   { return static_cast<NSView*>(w->get_hwnd()); }
NSWindow* get_nswindow(window* w) { return [get_nsview(w) window]; }

sciter::value window::get_glass() {
    NSWindow* nsw = get_nswindow(this);
    bool on = [nsw isOpaque] != YES;
    return sciter::value(on);
}
sciter::value window::set_glass(sciter::value on_off)
{
    NSWindow* nsw = get_nswindow(this);
    if( on_off.get(false) )
    {
        [nsw setOpaque:NO];
        NSColor* backgroundColor = [nsw backgroundColor];
        backgroundColor = [backgroundColor colorWithAlphaComponent:0.5];
        [nsw setBackgroundColor:backgroundColor];
    }
    else
    {
        [nsw setOpaque:YES];
    }
    
    sciter::value vars;
    vars.set_item(sciter::value("on-glass"), on_off);
    SciterSetMediaVars(get_hwnd(), &vars);
    
    return on_off;
}


// ugly: Global variable for sciter main frame because we can't access sciter from NSWindow itself.
window* g_main_window = nullptr;

void window::register_drop()
{
  // register drag-n-drop for the whole window
  NSWindow* window = get_nswindow(this);
  [window registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType, nil]];
  g_main_window = this;
}



@interface NSWindow(SciterWindow) <NSDraggingDestination>

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender;

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender;

@end

@implementation NSWindow(SciterWindow)

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
  return NSDragOperationGeneric;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
  
  NSPasteboard *pboard = [sender draggingPasteboard];
  
  if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
    NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
    if(files && files.count > 0 && g_main_window) {
      NSString* path = files.firstObject;
      
      aux::utf2w wsz(path.UTF8String);
      g_main_window->on_file_drop(wsz.c_str());
    }
  }
  return YES;
}

@end