/****************************************************************************
*    Copyright © 2014-2023 The Tumultuous Unicorn Of Darkness
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
****************************************************************************/

/*
* PROJECT CPU-X
* FILE gui_gtk.hpp
*/

#ifndef _GUI_GTK_HPP_
#define _GUI_GTK_HPP_

#include <gtkmm/window.h>
#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/drawingarea.h>
#include "data.hpp"

#define APPLICATION_ID "com.github." PRGUSERNAME "." PRGNAME_LOW // https://developer.gnome.org/documentation/tutorials/application-id.html#guidelines-for-choosing-an-application-id


enum GtkTheme
{
	AUTO,
	LIGHT,
	DARK
};

struct ExtTab : public Object
{
	Gtk::Label *name;

	ExtTab(Glib::RefPtr<Gtk::Builder> builder, const std::string name);
};
#define EXT_TAB(tab) static_cast<ExtTab*>(tab.ext)

struct ExtFrame : public Object
{
	Gtk::Label *name;

	ExtFrame(Glib::RefPtr<Gtk::Builder> builder, const std::string name);
};
#define EXT_FRAME(frame) static_cast<ExtFrame*>(frame.ext)

template <class T>
struct ExtLabel : public Object
{
	Gtk::Label *name;
	T *value;

	ExtLabel(Glib::RefPtr<Gtk::Builder> builder, const std::string name);
};
#define EXT_LABEL(label) static_cast<ExtLabel<Gtk::Label>*>(label.ext)
#define EXT_LABEL_PROGRESS_BAR(label) static_cast<ExtLabel<Gtk::ProgressBar>*>(label.ext)
#define EXT_LABEL_SWITCH(label) static_cast<ExtLabel<Gtk::Switch>*>(label.ext)
#define EXT_LABEL_SPIN_BUTTON(label) static_cast<ExtLabel<Gtk::SpinButton>*>(label.ext)

struct GtkData
{
	bool is_dark_theme = false;
	Data &data;

	/* Common */
	Gtk::Window *mainwindow = nullptr;
	Gtk::Button *daemonbutton = nullptr;
	Gtk::Label *labprgver = nullptr;
	Gtk::Notebook *notebook = nullptr;
	sigc::connection refresh_handle;

	/* Settings */
	Gtk::Window *settingswindow = nullptr;
	Gtk::Button *settingsbutton = nullptr;
	Gtk::Button *validatebutton = nullptr;
	Gtk::Button *cancelbutton = nullptr;
	Gtk::ComboBoxText *temperatureunit = nullptr;
	Gtk::SpinButton *refreshtime = nullptr;
	Gtk::ComboBoxText *theme = nullptr;
	Gtk::ComboBoxText *defaulttab = nullptr;
	Gtk::ComboBoxText *defaulttype = nullptr;
	Gtk::ComboBoxText *defaultcore = nullptr;
	Gtk::ComboBoxText *defaultcachetest = nullptr;
	Gtk::ComboBoxText *defaultstick = nullptr;
	Gtk::ComboBoxText *defaultcard = nullptr;
	Gtk::CheckButton *cpuiddecimal = nullptr;
	Gtk::CheckButton *startdaemon = nullptr;

	GtkData(Glib::RefPtr<Gtk::Builder> builder, Data &data);
	void get_widgets(Glib::RefPtr<Gtk::Builder> builder);
	void check_theme_color();
	void set_colors();
	void set_logos();
	void set_all_labels();
	void set_signals();
	void bind_settings();
	bool grefresh();

	/* Tab CPU */
	struct ExtTabCpu : public ExtTab
	{
		Gtk::Image *logocpu = nullptr;
		Gtk::ComboBoxText *activetype = nullptr;
		Gtk::ComboBoxText *activecore = nullptr;

		ExtTabCpu(Glib::RefPtr<Gtk::Builder> builder);
	};
#define EXT_TAB_CPU(cpu) static_cast<ExtTabCpu*>(cpu.ext)
	void gtab_cpu();
	void fill_box_active_type();
	void fill_box_active_type(Gtk::ComboBoxText *comboboxtext);
	void fill_box_active_core();
	void fill_box_active_core(Gtk::ComboBoxText *comboboxtext);
	void change_active_type();
	void change_active_core();

	/* Tab Caches */
	struct ExtTabCaches : public ExtTab
	{
		Gtk::Grid *gridcaches = nullptr;
		Gtk::ComboBoxText *activetest = nullptr;

		ExtTabCaches(Glib::RefPtr<Gtk::Builder> builder);
	};
#define EXT_TAB_CACHES(caches) static_cast<ExtTabCaches*>(caches.ext)
	void gtab_caches();
	void fill_box_active_test();
	void fill_box_active_test(Gtk::ComboBoxText *comboboxtext);
	void change_active_test();

	/* Tab Motherboard */
	struct ExtTabMotherboard : public ExtTab
	{
		ExtTabMotherboard(Glib::RefPtr<Gtk::Builder> builder);
	};
	void gtab_motherboard();

	/* Tab Memory */
	struct ExtTabMemory : public ExtTab
	{
		Gtk::ScrolledWindow *scrolledsticks = nullptr;
		Gtk::ComboBoxText *activestick = nullptr;

		ExtTabMemory(Glib::RefPtr<Gtk::Builder> builder);
	};
#define EXT_TAB_MEMORY(memory) static_cast<ExtTabMemory*>(memory.ext)
	void gtab_memory();
	void fill_box_active_stick();
	void fill_box_active_stick(Gtk::ComboBoxText *comboboxtext);
	void change_active_stick();

	/* Tab System */
	struct ExtTabSystem : public ExtTab
	{
		Gtk::DrawingArea *barused = nullptr;
		Gtk::DrawingArea *barbuff = nullptr;
		Gtk::DrawingArea *barcache = nullptr;
		Gtk::DrawingArea *barfree = nullptr;
		Gtk::DrawingArea *barswap = nullptr;

		ExtTabSystem(Glib::RefPtr<Gtk::Builder> builder);
	};
#define EXT_TAB_SYSTEM(system) static_cast<ExtTabSystem*>(system.ext)
	void gtab_system();
	bool draw_bar_memory_used(const Cairo::RefPtr<Cairo::Context> &cr);
	bool draw_bar_memory_buffers(const Cairo::RefPtr<Cairo::Context> &cr);
	bool draw_bar_memory_cached(const Cairo::RefPtr<Cairo::Context> &cr);
	bool draw_bar_memory_free(const Cairo::RefPtr<Cairo::Context> &cr);
	bool draw_bar_memory_swap(const Cairo::RefPtr<Cairo::Context> &cr);
	bool draw_bar_memory(const Cairo::RefPtr<Cairo::Context> &cr, Gtk::DrawingArea *area, Gtk::Label *label, double before, double percent);

	/* Tab Graphics */
	struct ExtTabGraphics : public ExtTab
	{
		Gtk::ScrolledWindow *scrolledcards = nullptr;
		Gtk::ComboBoxText *activecard = nullptr;

		ExtTabGraphics(Glib::RefPtr<Gtk::Builder> builder);
	};
#define EXT_TAB_GRAPHICS(graphics) static_cast<ExtTabGraphics*>(graphics.ext)
	void gtab_graphics();
	void fill_box_active_card();
	void fill_box_active_card(Gtk::ComboBoxText *comboboxtext);
	void change_active_card();

	/* Tab Bench */
	struct ExtTabBench : public ExtTab
	{
		ExtTabBench(Glib::RefPtr<Gtk::Builder> builder);
	};
#define EXT_TAB_BENCH(bench) static_cast<ExtTabBench*>(bench.ext)
	void gtab_bench();
	bool start_bench_prime_slow(bool state);
	bool start_bench_prime_fast(bool state);
	void change_bench_sensitive(bool state);
	void change_bench_duration();
	void change_bench_threads();

	/* Tab About */
	struct ExtTabAbout : public ExtTab
	{
		Gtk::Image *logoprg = nullptr;
		Gtk::Label *about_descr = nullptr;
		Gtk::Label *about_version = nullptr;
		Gtk::Label *about_author = nullptr;
		Gtk::Label *about_website = nullptr;
		Gtk::Label *license_copyright = nullptr;
		Gtk::Label *license_name = nullptr;
		Gtk::Label *license_warranty = nullptr;

		ExtTabAbout(Glib::RefPtr<Gtk::Builder> builder);
	};
#define EXT_TAB_ABOUT(about) static_cast<ExtTabAbout*>(about.ext)
	void gtab_about();
};

struct SettingsWindow
{


	SettingsWindow(Glib::RefPtr<Gtk::Builder> builder, Data &data);
	void get_widgets(Glib::RefPtr<Gtk::Builder> builder);
	void set_signals();
};


#endif /* _GUI_GTK_HPP_ */
