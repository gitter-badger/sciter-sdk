
#include "sciter-x-window.hpp"
#include "behaviors/behavior_video_generator.cpp"

// native API demo

// native functions exposed to script

static int native_get_element_uid(sciter::value vel) {
  sciter::dom::element el = (HELEMENT)vel.get_object_data();
  return (int)el.get_element_uid();
}


static sciter::value native_api() {

  sciter::value api_map;

  api_map.set_item(sciter::value("getElementUid"), sciter::vfunc( native_get_element_uid ) );
  return api_map;

  /* the above returns this:
    return {
      getElementUid(): {native_get_element_uid}
    }
  */

}

class frame: public sciter::window {
public:
  frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_GLASSY | SW_ENABLE_DEBUG)
  {
    register_drop();
  }

  BEGIN_FUNCTION_MAP
    FUNCTION_0("nativeApi", native_api);
  END_FUNCTION_MAP

  void register_drop();

  void on_file_drop(const sciter::string& file)
  {
    this->call_function("loadFile", file);
  }
};

#include "resources.cpp"

int uimain(std::function<int()> run ) {

  //sciter::debug_output_console console; - uncomment it if you will need console window

  sciter::archive::instance().open(aux::elements_of(resources)); // bind resources[] (defined in "resources.cpp") with the archive

  frame *pwin = new frame();

  // note: this:://app URL is dedicated to the sciter::archive content associated with the application
  pwin->load( WSTR("this://app/default.htm") );

  pwin->expand();

  return run();
}

#define LINUX

#ifdef LINUX
#include <gtk/gtk.h>
#include <gdk/gdk.h>


static void
drag_data_received (GtkWidget          *widget,
                    GdkDragContext     *context,
                    gint                x,
                    gint                y,
                    GtkSelectionData   *sel,
                    guint               info,
                    guint               time,
                    gpointer            param)
{
  gint length = gtk_selection_data_get_length(sel);
  if(length != -1) {
    char* data = (char*)gtk_selection_data_get_text(sel);
    //char* type = gdk_atom_name(gtk_selection_data_get_data_type(sel));
    //printf("drag data '%s' (%d): <%s>\n", type, length, data);

    aux::chars file(data, length);
    while(file.length && isspace(file.last()))  // cut \r\n from file uri string
      file.prune(0, 1);

    if(frame* wnd = reinterpret_cast<frame*>(param)) {
      const aux::utf2w wfile(file.start, file.length);
      wnd->on_file_drop(wfile.c_str());
    }
  }
  gdk_drop_finish(context, true, time);
}

void frame::register_drop()
{
  //sciter::string exe = sciter::application::argv().front();
  //exe += aux::utf2w(".ico.png");

  GtkWidget* widget = this->get_hwnd();
  GtkWidget* top = gtk_widget_get_toplevel(widget);
  GtkWindow* window = GTK_WINDOW(top);

  // set window icon from file
  //gtk_window_set_icon_from_file(window, aux::w2utf(exe).c_str(), NULL);

  // setup drag&drop
  gtk_drag_dest_set(widget, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
  gtk_drag_dest_add_text_targets(widget);
  g_signal_connect(widget, "drag_data_received", G_CALLBACK(drag_data_received), this);
}

#endif // LINUX
