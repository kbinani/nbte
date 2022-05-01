// Dear ImGui: standalone example application for OSX + Metal.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#import <Foundation/Foundation.h>

#import <Cocoa/Cocoa.h>

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "imgui.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_osx.h"
@interface AppViewController : NSViewController
@end
#include "imgui_stdlib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <hwm/task/task_queue.hpp>
#include <minecraft-file.hpp>
#include <nfd.h>
#include <variant>

#include "resource/udev_gothic35_regular.h"
#include "resource/nbte32.h"
#include "version.hpp"
#include "string.hpp"
#include "model/node.hpp"
#include "model/node.impl.hpp"
#include "model/state.hpp"
#include "platform.hpp"
#include "render/legal.hpp"
#include "render/render.hpp"

@interface AppViewController () <MTKViewDelegate>
@property (nonatomic, readonly) MTKView *mtkView;
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;
@end

//-----------------------------------------------------------------------------------
// AppViewController
//-----------------------------------------------------------------------------------

@implementation AppViewController {
    nbte::State state;
}

-(instancetype)initWithNibName:(nullable NSString *)nibNameOrNil bundle:(nullable NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    
    _device = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];
    
    if (!self.device)
    {
        NSLog(@"Metal is not supported");
        abort();
    }
    
    // Setup Dear ImGui context
    // FIXME: This example doesn't have proper cleanup...
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.IniFilename = nullptr;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    
    ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(udev_gothic35_regular_compressed_data, udev_gothic35_regular_compressed_size, 15.0f);
    
    // Setup Renderer backend
    ImGui_ImplMetal_Init(_device);
    
    return self;
}

-(MTKView *)mtkView
{
    return (MTKView *)self.view;
}

-(void)loadView
{
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, 1200, 720)];
}

-(void)viewDidLoad
{
    [super viewDidLoad];
    
    self.mtkView.device = self.device;
    self.mtkView.delegate = self;
    
    // Add a tracking area in order to receive mouse events whenever the mouse is within the bounds of our view
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                                options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways
                                                                  owner:self
                                                               userInfo:nil];
    [self.view addTrackingArea:trackingArea];
    
    ImGui_ImplOSX_Init(self.view);
}

-(void)drawInMTKView:(MTKView*)view
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = view.bounds.size.width;
    io.DisplaySize.y = view.bounds.size.height;
    
    CGFloat framebufferScale = view.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
    io.DisplayFramebufferScale = ImVec2(framebufferScale, framebufferScale);
    
    io.DeltaTime = 1 / float(view.preferredFramesPerSecond ?: 60);
    
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
    
    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
    if (renderPassDescriptor == nil)
    {
        [commandBuffer commit];
        return;
    }
    
    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(renderPassDescriptor);
    ImGui_ImplOSX_NewFrame(view);
    
    ImGui::NewFrame();
    
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
    self->state.fDisplaySize = io.DisplaySize;
    nbte::Render(state);
    
    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"Dear ImGui rendering"];
    ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];
    
    // Present
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
    
    NSWindow *window = [[NSApplication sharedApplication] keyWindow];
    NSString *title = [[NSString alloc] initWithUTF8String:self->state.winowTitle().c_str()];
    if (window && window.title != title) {
        window.title = title;
    }
    
    if (state.fMainMenuBarQuitSelected) {
        [[NSApplication sharedApplication] terminate:self];
    }
}

-(void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

//-----------------------------------------------------------------------------------
// Input processing
//-----------------------------------------------------------------------------------

// Forward Mouse events to Dear ImGui OSX backend.
-(void)mouseDown:(NSEvent *)event           { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseDown:(NSEvent *)event      { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseDown:(NSEvent *)event      { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseUp:(NSEvent *)event             { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseUp:(NSEvent *)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseUp:(NSEvent *)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseMoved:(NSEvent *)event          { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseDragged:(NSEvent *)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseMoved:(NSEvent *)event     { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseDragged:(NSEvent *)event   { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseMoved:(NSEvent *)event     { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseDragged:(NSEvent *)event   { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)scrollWheel:(NSEvent *)event         { ImGui_ImplOSX_HandleEvent(event, self.view); }

@end

//-----------------------------------------------------------------------------------
// AppDelegate
//-----------------------------------------------------------------------------------

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@property (strong) NSWindowController *windowController;
@end

@implementation AppDelegate

-(instancetype)init
{
    if (self = [super init])
    {
        NSViewController *rootViewController = [[AppViewController alloc] initWithNibName:nil bundle:nil];
        self.window = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                    backing:NSBackingStoreBuffered
                                                      defer:NO];
        self.window.contentViewController = rootViewController;
    }
    return self;
}

-(void)applicationDidFinishLaunching:(NSNotification *)notification
{
    _windowController = [[NSWindowController alloc] initWithWindowNibName:@"MainMenu"];
    [_windowController showWindow:self];

    [self.window orderFront:self];
    [self.window center];
    [self.window becomeKeyWindow];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

@end

//-----------------------------------------------------------------------------------
// Application main() function
//-----------------------------------------------------------------------------------

int main(int argc, const char * argv[])
{
    @autoreleasepool
    {
        NSApp = [NSApplication sharedApplication];
        AppDelegate* delegate = [[AppDelegate alloc] init];
        [[NSApplication sharedApplication] setDelegate:delegate];
        [NSApp run];
    }
    return NSApplicationMain(argc, argv);
}
