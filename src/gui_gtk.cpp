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
* FILE gui_gtk.cpp
*/

#include <unistd.h>
#include <gtkmm.h>
#include <gtkmm/messagedialog.h>
#include <giomm/settings.h>
#include <sys/wait.h>
#include <iostream>
#include "options.hpp"
#include "util.hpp"
#include "data.hpp"
#include "core.hpp"
#include "daemon.h"
#include "daemon_client.hpp"
#include "gui_gtk.hpp"

using CacheLevels = Data::Caches::CpuType::CacheLevels;


/************************* Private functions *************************/

Glib::RefPtr<Gtk::Application> app;
static Glib::RefPtr<Gio::Settings> settings;

/* Search file location in standard paths */
static const std::string get_data_path(const std::string &file)
{
	for(const auto& dir : Glib::get_system_data_dirs())
	{
		const std::string full_path = dir + "/" + PRGNAME_LOW + "/" + file;
		if(Glib::file_test(full_path, Glib::FILE_TEST_EXISTS))
		{
			MSG_DEBUG("get_data_path: file=%s ==> %s found", file.c_str(), full_path.c_str());
			return full_path;
		}
		else
			MSG_DEBUG("get_data_path: file=%s ==> %s does not exist", file.c_str(), full_path.c_str());
	}

	MSG_ERROR(_("Cannot find path for '%s' file"), file.c_str());
	return std::string();
}

static void set_tab_name(const Tab &tab)
{
	EXT_TAB(tab)->name->set_text(tab.name);
}

static void set_frame_name(const Frame &frame)
{
	EXT_FRAME(frame)->name->set_text(frame.name);
}

static void set_label_name_and_value(const Label &label)
{
	EXT_LABEL(label)->name->set_text(label.name);
	EXT_LABEL(label)->value->set_text(label.value);
}

static void set_label_name_and_value(const Label &label, const std::string tooltip)
{
	set_label_name_and_value(label);
	EXT_LABEL(label)->name->set_tooltip_text(_(tooltip.c_str()));
}


/* Object-derived classes */

ExtTab::ExtTab(Glib::RefPtr<Gtk::Builder> builder, const std::string name)
{
	builder->get_widget(name, this->name);
	this->name->set_name("tab");
}

ExtFrame::ExtFrame(Glib::RefPtr<Gtk::Builder> builder, const std::string name)
{
	builder->get_widget(name, this->name);
	this->name->set_name(name);
}

template <class T>
ExtLabel<T>::ExtLabel(Glib::RefPtr<Gtk::Builder> builder, const std::string name)
{
	const auto index = name.find("_") + 1;
	const std::string prefix = name.substr(0, index);
	const std::string suffix = name.substr(index);

	builder->get_widget(prefix + "lab" + suffix, this->name);
	builder->get_widget(prefix + "val" + suffix, this->value);
	this->value->set_name("value");
}


/* GtkData class */

GtkData::GtkData(Glib::RefPtr<Gtk::Builder> builder, Data &data) : data(data)
{
	this->get_widgets(builder);
	this->check_theme_color();
	this->set_colors();
	this->set_logos();
	this->set_all_labels();
	this->set_signals();
	this->bind_settings();
}

void GtkData::get_widgets(Glib::RefPtr<Gtk::Builder> builder)
{
	/* Common */
	builder->get_widget("mainwindow", this->mainwindow);
	builder->get_widget("daemonbutton", this->daemonbutton);
	builder->get_widget("labprgver", this->labprgver);
	builder->get_widget("header_notebook", this->notebook);

	/* Settings */
	builder->get_widget("settingswindow", this->settingswindow);
	builder->get_widget("settingsbutton", this->settingsbutton);
	builder->get_widget("validatebutton", this->validatebutton);
	builder->get_widget("cancelbutton", this->cancelbutton);
	builder->get_widget("temperatureunit_val", this->temperatureunit);
	builder->get_widget("refreshtime_val", this->refreshtime);
	builder->get_widget("theme_val", this->theme);
	builder->get_widget("defaulttab_val", this->defaulttab);
	builder->get_widget("defaulttype_val", this->defaulttype);
	builder->get_widget("defaultcore_val", this->defaultcore);
	builder->get_widget("defaultcachetest_val", this->defaultcachetest);
	builder->get_widget("defaultstick_val", this->defaultstick);
	builder->get_widget("defaultcard_val", this->defaultcard);
	builder->get_widget("cpuiddecimal", this->cpuiddecimal);
	builder->get_widget("startdaemon", this->startdaemon);

	/* Tab: CPU */
	this->data.cpu.extend(new GtkData::ExtTabCpu(builder));
	for(auto& cpu_type : this->data.cpu.cpu_types)
	{
		/* Processor frame */
		cpu_type.processor.extend(new ExtFrame(builder, "proc_lab"));
		cpu_type.processor.vendor.       extend(new ExtLabel<Gtk::Label>(builder, "proc_vendor"));
		cpu_type.processor.codename.     extend(new ExtLabel<Gtk::Label>(builder, "proc_cdename"));
		cpu_type.processor.package.      extend(new ExtLabel<Gtk::Label>(builder, "proc_pkg"));
		cpu_type.processor.technology.   extend(new ExtLabel<Gtk::Label>(builder, "proc_tech"));
		cpu_type.processor.voltage.      extend(new ExtLabel<Gtk::Label>(builder, "proc_volt"));
		cpu_type.processor.specification.extend(new ExtLabel<Gtk::Label>(builder, "proc_spec"));
		cpu_type.processor.family.       extend(new ExtLabel<Gtk::Label>(builder, "proc_fam"));
		cpu_type.processor.dispfamily.   extend(new ExtLabel<Gtk::Label>(builder, "proc_extfam"));
		cpu_type.processor.model.        extend(new ExtLabel<Gtk::Label>(builder, "proc_mod"));
		cpu_type.processor.dispmodel.    extend(new ExtLabel<Gtk::Label>(builder, "proc_extmod"));
		cpu_type.processor.temperature.  extend(new ExtLabel<Gtk::Label>(builder, "proc_temp"));
		cpu_type.processor.stepping.     extend(new ExtLabel<Gtk::Label>(builder, "proc_step"));
		cpu_type.processor.instructions. extend(new ExtLabel<Gtk::Label>(builder, "proc_instr"));
		/* Caches frame */
		cpu_type.caches.extend(new ExtFrame(builder, "cache_lab"));
		cpu_type.caches.level1d.extend(new ExtLabel<Gtk::Label>(builder, "cache_l1d"));
		cpu_type.caches.level1i.extend(new ExtLabel<Gtk::Label>(builder, "cache_l1i"));
		cpu_type.caches.level2. extend(new ExtLabel<Gtk::Label>(builder, "cache_l2"));
		cpu_type.caches.level3. extend(new ExtLabel<Gtk::Label>(builder, "cache_l3"));
		/* Footer frame */
		cpu_type.footer.cores.  extend(new ExtLabel<Gtk::Label>(builder, "cpufooter_core"));
		cpu_type.footer.threads.extend(new ExtLabel<Gtk::Label>(builder, "cpufooter_thrd"));
	}
	/* Clocks frame */
	this->data.cpu.clocks.extend(new ExtFrame(builder, "clock_lab"));
	this->data.cpu.clocks.core_speed.extend(new ExtLabel<Gtk::Label>(builder, "clock_core"));
	this->data.cpu.clocks.multiplier.extend(new ExtLabel<Gtk::Label>(builder, "clock_mult"));
	this->data.cpu.clocks.bus_speed. extend(new ExtLabel<Gtk::Label>(builder, "clock_bus"));
	this->data.cpu.clocks.usage.     extend(new ExtLabel<Gtk::Label>(builder, "clock_usage"));

	/* Tab: Caches */
	this->data.caches.extend(new GtkData::ExtTabCaches(builder));
	for(auto& cpu_type : this->data.caches.cpu_types)
	{
		/* Cache frame */
		for(unsigned index = 0; index < cpu_type.caches.size(); index++)
		{
			const std::string level_string = "l" + std::to_string(index + 1);
			cpu_type.caches[index].extend(new ExtFrame(builder, level_string + "cache_lab"));
			cpu_type.caches[index].size. extend(new ExtLabel<Gtk::Label>(builder, level_string + "cache_size"));
			cpu_type.caches[index].speed.extend(new ExtLabel<Gtk::Label>(builder, level_string + "cache_speed"));
		}
	}
	/* Test frame */
	this->data.caches.test.extend(new ExtFrame(builder, "test_lab"));

	/* Tab: Motherboard */
	this->data.motherboard.extend(new GtkData::ExtTabMotherboard(builder));
	/* Motherboard frame */
	this->data.motherboard.board.extend(new ExtFrame(builder, "motherboard_lab"));
	this->data.motherboard.board.manufacturer.extend(new ExtLabel<Gtk::Label>(builder, "motherboard_manu"));
	this->data.motherboard.board.model.       extend(new ExtLabel<Gtk::Label>(builder, "motherboard_mod"));
	this->data.motherboard.board.revision.    extend(new ExtLabel<Gtk::Label>(builder, "motherboard_rev"));
	/* BIOS frame */
	this->data.motherboard.bios.extend(new ExtFrame(builder, "bios_lab"));
	this->data.motherboard.bios.brand.  extend(new ExtLabel<Gtk::Label>(builder, "bios_brand"));
	this->data.motherboard.bios.version.extend(new ExtLabel<Gtk::Label>(builder, "bios_vers"));
	this->data.motherboard.bios.date.   extend(new ExtLabel<Gtk::Label>(builder, "bios_date"));
	this->data.motherboard.bios.romsize.extend(new ExtLabel<Gtk::Label>(builder, "bios_rom"));
	/* Chipset frame */
	this->data.motherboard.chipset.extend(new ExtFrame(builder, "chip_lab"));
	this->data.motherboard.chipset.vendor.extend(new ExtLabel<Gtk::Label>(builder, "chip_vend"));
	this->data.motherboard.chipset.model. extend(new ExtLabel<Gtk::Label>(builder, "chip_name"));

	/* Tab: Memory */
	this->data.memory.extend(new GtkData::ExtTabMemory(builder));
	/* Stick frame */
	for(auto& stick : this->data.memory.sticks)
	{
		stick.extend(new ExtFrame(builder, "stick_lab"));
		stick.manufacturer.  extend(new ExtLabel<Gtk::Label>(builder, "stick_manufacturer"));
		stick.part_number.   extend(new ExtLabel<Gtk::Label>(builder, "stick_partnumber"));
		stick.type.          extend(new ExtLabel<Gtk::Label>(builder, "stick_type"));
		stick.type_detail.   extend(new ExtLabel<Gtk::Label>(builder, "stick_typedetail"));
		stick.device_locator.extend(new ExtLabel<Gtk::Label>(builder, "stick_devicelocator"));
		stick.bank_locator.  extend(new ExtLabel<Gtk::Label>(builder, "stick_banklocator"));
		stick.size.          extend(new ExtLabel<Gtk::Label>(builder, "stick_size"));
		stick.rank.          extend(new ExtLabel<Gtk::Label>(builder, "stick_rank"));
		stick.speed.         extend(new ExtLabel<Gtk::Label>(builder, "stick_speed"));
		stick.voltage.       extend(new ExtLabel<Gtk::Label>(builder, "stick_voltage"));
	}
	/* Sticks frame */
	this->data.memory.footer.extend(new ExtFrame(builder, "sticks_lab"));

	/* Tab: System */
	this->data.system.extend(new GtkData::ExtTabSystem(builder));
	/* Operating System frame */
	this->data.system.os.extend(new ExtFrame(builder, "os_lab"));
	this->data.system.os.name.    extend(new ExtLabel<Gtk::Label>(builder, "os_name"));
	this->data.system.os.kernel.  extend(new ExtLabel<Gtk::Label>(builder, "os_kernel"));
	this->data.system.os.hostname.extend(new ExtLabel<Gtk::Label>(builder, "os_hostname"));
	this->data.system.os.uptime.  extend(new ExtLabel<Gtk::Label>(builder, "os_uptime"));
	/* Memory frame */
	this->data.system.memory.extend(new ExtFrame(builder, "mem_lab"));
	this->data.system.memory.used.   extend(new ExtLabel<Gtk::Label>(builder, "mem_used"));
	this->data.system.memory.buffers.extend(new ExtLabel<Gtk::Label>(builder, "mem_buff"));
	this->data.system.memory.cached. extend(new ExtLabel<Gtk::Label>(builder, "mem_cache"));
	this->data.system.memory.free.   extend(new ExtLabel<Gtk::Label>(builder, "mem_free"));
	this->data.system.memory.swap.   extend(new ExtLabel<Gtk::Label>(builder, "mem_swap"));

	/* Tab: Graphics */
	this->data.graphics.extend(new GtkData::ExtTabGraphics(builder));
	/* Card frame */
	for(auto& card : this->data.graphics.cards)
	{
		card.extend(new ExtFrame(builder, "card_lab"));
		card.vendor.          extend(new ExtLabel<Gtk::Label>(builder, "card_vend"));
		card.kernel_driver.   extend(new ExtLabel<Gtk::Label>(builder, "card_driver"));
		card.user_mode_driver.extend(new ExtLabel<Gtk::Label>(builder, "card_umd"));
		card.model.           extend(new ExtLabel<Gtk::Label>(builder, "card_mod"));
		card.comp_unit.       extend(new ExtLabel<Gtk::Label>(builder, "card_comp_unit"));
		card.device_id.       extend(new ExtLabel<Gtk::Label>(builder, "card_did_rid"));
		card.vbios_version.   extend(new ExtLabel<Gtk::Label>(builder, "card_vbios_ver"));
		card.interface.       extend(new ExtLabel<Gtk::Label>(builder, "card_pcie"));
		card.temperature.     extend(new ExtLabel<Gtk::Label>(builder, "card_temp"));
		card.usage.           extend(new ExtLabel<Gtk::Label>(builder, "card_usage"));
		card.core_voltage.    extend(new ExtLabel<Gtk::Label>(builder, "card_gvolt"));
		card.power_avg.       extend(new ExtLabel<Gtk::Label>(builder, "card_gpwr"));
		card.core_clock.      extend(new ExtLabel<Gtk::Label>(builder, "card_gclk"));
		card.mem_clock.       extend(new ExtLabel<Gtk::Label>(builder, "card_mclk"));
		card.mem_used.        extend(new ExtLabel<Gtk::Label>(builder, "card_vram_used"));
		card.resizable_bar.   extend(new ExtLabel<Gtk::Label>(builder, "card_rebar"));
		card.vulkan_rt.       extend(new ExtLabel<Gtk::Label>(builder, "card_vkrt"));
		card.opengl_version.  extend(new ExtLabel<Gtk::Label>(builder, "card_gl_ver"));
		card.vulkan_version.  extend(new ExtLabel<Gtk::Label>(builder, "card_vk_ver"));
		card.opencl_version.  extend(new ExtLabel<Gtk::Label>(builder, "card_cl_ver"));
	}
	/* Cards frame */
	this->data.graphics.footer.extend(new ExtFrame(builder, "cards_lab"));

	/* Tab: Bench */
	this->data.bench.extend(new GtkData::ExtTabBench(builder));
	/* Prime Slow frame */
	this->data.bench.prime_slow.extend(new ExtFrame(builder, "primeslow_lab"));
	this->data.bench.prime_slow.score.extend(new ExtLabel<Gtk::ProgressBar>(builder, "primeslow_score"));
	this->data.bench.prime_slow.state.extend(new ExtLabel<Gtk::Switch>(builder, "primeslow_run"));
	/* Prime Fast frame */
	this->data.bench.prime_fast.extend(new ExtFrame(builder, "primefast_lab"));
	this->data.bench.prime_fast.score.extend(new ExtLabel<Gtk::ProgressBar>(builder, "primefast_score"));
	this->data.bench.prime_fast.state.extend(new ExtLabel<Gtk::Switch>(builder, "primefast_run"));
	/* Parameters frame */
	this->data.bench.parameters.extend(new ExtFrame(builder, "param_lab"));
	this->data.bench.parameters.duration.extend(new ExtLabel<Gtk::SpinButton>(builder, "param_duration"));
	this->data.bench.parameters.threads. extend(new ExtLabel<Gtk::SpinButton>(builder, "param_threads"));

	/* Tab: About */
	this->data.about.extend(new GtkData::ExtTabAbout(builder));
	/* About frame */
	this->data.about.about.extend(new ExtFrame(builder, "about_lab"));
	/* License frame */
	this->data.about.license.extend(new ExtFrame(builder, "license_lab"));
}

void GtkData::check_theme_color()
{
	const GtkTheme current_theme = static_cast<GtkTheme>(settings->get_enum("gui-theme"));
	if(current_theme == AUTO)
	{
		const auto stylecontext = this->mainwindow->get_style_context();
		const auto fg = stylecontext->get_color(Gtk::STATE_FLAG_NORMAL);
		const auto bg = stylecontext->get_background_color(Gtk::STATE_FLAG_NORMAL);
		const double contrast = bg.get_red() - fg.get_red() + bg.get_green() - fg.get_green() + bg.get_blue() - fg.get_blue();
		this->is_dark_theme = (contrast < -1);
	}
	else
		this->is_dark_theme = (current_theme == DARK);
}

void GtkData::set_colors()
{
 	auto provider = Gtk::CssProvider::create();

#if GTK_CHECK_VERSION(3, 20, 0) // GTK 3.20+
	provider->load_from_path(get_data_path(this->is_dark_theme ? "cpu-x-gtk-3.20-dark.css" : "cpu-x-gtk-3.20.css"));
#else // GTK 3.12 to 3.18
	provider->load_from_path(get_data_path(this->is_dark_theme ? "cpu-x-gtk-3.12-dark.css" : "cpu-x-gtk-3.12.css"));
#endif
	Gtk::StyleContext::add_provider_for_screen(Gdk::Screen::get_default(), provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void GtkData::set_logos()
{
	bool set_unknown     = false;
	const auto& cpu_type = this->data.cpu.get_selected_cpu_type();
	const int prg_size   = 72;
	const int width      = EXT_LABEL(cpu_type.processor.specification)->value->get_allocated_width() - EXT_LABEL(cpu_type.processor.vendor)->value->get_allocated_width() - 6;
	const int height     = (EXT_LABEL(cpu_type.processor.vendor)->value->get_allocated_height() + 4) * 4;

	/* CPU logo */
	try
	{
		if(cpu_type.processor.vendor.value.size() > 0)
		{
			auto cpu_pixbuf = Gdk::Pixbuf::create_from_file(get_data_path(cpu_type.processor.vendor.value + ".png"), width, height, true);
			EXT_TAB_CPU(this->data.cpu)->logocpu->set(cpu_pixbuf);
		}
		else
			set_unknown = true;
	}
	catch(...)
	{
		set_unknown = true;
	}

	try
	{
		if(set_unknown)
		{
			auto unknown_pixbuf = Gdk::Pixbuf::create_from_file(get_data_path("Unknown.png"), width, height, true);
			EXT_TAB_CPU(this->data.cpu)->logocpu->set(unknown_pixbuf);
		}
	}
	catch(...)
	{
		MSG_ERRNO("%s", "failed to set CPU icon");
	}

	/* Program logo */
	try
	{
		auto prg_pixbuf = Gdk::Pixbuf::create_from_file(get_data_path(std::string(PRGNAME) + ".png"), prg_size, prg_size, true);
		this->mainwindow->set_icon(prg_pixbuf);
		EXT_TAB_ABOUT(this->data.about)->logoprg->set(prg_pixbuf);
	}
	catch(...)
	{
		MSG_ERRNO("%s", "failed to set program icon");
	}
}

void GtkData::set_all_labels()
{
	Gtk::Requisition minimum_size, natural_size;
	const auto margin     = 6;
	const auto width_full = EXT_LABEL(this->data.system.os.hostname)->value->get_allocated_width();
	const auto width_half = width_full - EXT_LABEL(this->data.system.memory.used)->value->get_allocated_width() - margin;
	const int pkcheck     = run_command("pkcheck --action-id org.freedesktop.policykit.exec --process %u > /dev/null 2>&1", getpid());

	/* Common */
	notebook->set_current_page(Options::get_selected_page());
	this->labprgver->set_text(this->data.about.about.version);
	this->daemonbutton->set_label(_("Start daemon"));
	this->daemonbutton->set_sensitive(false);
	if(DAEMON_UP)
		this->daemonbutton->set_tooltip_text(_("Connected to daemon"));
	else if(WEXITSTATUS(pkcheck) > 2)
		this->daemonbutton->set_tooltip_text(_("No polkit authentication agent found"));
	else
	{
		this->daemonbutton->set_sensitive(true);
		this->daemonbutton->set_tooltip_text(_("Ask password to start daemon in background"));
	}

	/* Settings */
	this->settingswindow->set_title(_("Settings"));
	this->refreshtime->set_range(1, G_MAXUSHORT);
	this->refreshtime->set_increments(1, 60);
	this->defaulttab->insert(0, "cpu",        this->data.cpu.name);
	this->defaulttab->insert(1, "caches",     this->data.caches.name);
	this->defaulttab->insert(2, "motherboad", this->data.motherboard.name);
	this->defaulttab->insert(3, "memory",     this->data.memory.name);
	this->defaulttab->insert(4, "system",     this->data.system.name);
	this->defaulttab->insert(5, "graphics",   this->data.graphics.name);
	this->defaulttab->insert(6, "bench",      this->data.bench.name);
	this->defaulttab->insert(7, "about",      this->data.about.name);
	this->defaulttab->set_active(Options::get_selected_page());
	this->fill_box_active_type(this->defaulttype);
	this->fill_box_active_core(this->defaultcore);
	this->fill_box_active_test(this->defaultcachetest);
	this->fill_box_active_stick(this->defaultstick);
	this->fill_box_active_card(this->defaultcard);

	/* Tab: system */
	EXT_LABEL(this->data.system.memory.used)->value->get_preferred_size(minimum_size, natural_size);
	EXT_TAB_SYSTEM(this->data.system)->barused-> set_size_request(width_half, natural_size.height);
	EXT_TAB_SYSTEM(this->data.system)->barbuff-> set_size_request(width_half, natural_size.height);
	EXT_TAB_SYSTEM(this->data.system)->barcache->set_size_request(width_half, natural_size.height);
	EXT_TAB_SYSTEM(this->data.system)->barfree-> set_size_request(width_half, natural_size.height);
	EXT_TAB_SYSTEM(this->data.system)->barswap-> set_size_request(width_half, natural_size.height);
	EXT_TAB_SYSTEM(this->data.system)->barused-> signal_draw().connect(sigc::mem_fun(*this, &GtkData::draw_bar_memory_used));
	EXT_TAB_SYSTEM(this->data.system)->barbuff-> signal_draw().connect(sigc::mem_fun(*this, &GtkData::draw_bar_memory_buffers));
	EXT_TAB_SYSTEM(this->data.system)->barcache->signal_draw().connect(sigc::mem_fun(*this, &GtkData::draw_bar_memory_cached));
	EXT_TAB_SYSTEM(this->data.system)->barfree-> signal_draw().connect(sigc::mem_fun(*this, &GtkData::draw_bar_memory_free));
	EXT_TAB_SYSTEM(this->data.system)->barswap-> signal_draw().connect(sigc::mem_fun(*this, &GtkData::draw_bar_memory_swap));

	/* Tab: Bench */
	EXT_LABEL_PROGRESS_BAR(this->data.bench.prime_slow.score)->value->set_size_request(width_full, natural_size.height);
	EXT_LABEL_PROGRESS_BAR(this->data.bench.prime_fast.score)->value->set_size_request(width_full, natural_size.height);
	EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.duration)->value->set_increments(1, 60);
	EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.duration)->value->set_range(1, 60 * 24);
	EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.threads)->value->set_increments(1, 1);
	EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.threads)->value->set_range(1, std::thread::hardware_concurrency());

	/* All tabs */
	this->gtab_cpu();
	this->fill_box_active_type();
	this->fill_box_active_core();
	this->gtab_caches();
	this->fill_box_active_test();
	this->gtab_motherboard();
	this->gtab_memory();
	this->fill_box_active_stick();
	this->gtab_system();
	this->gtab_graphics();
	this->fill_box_active_card();
	this->gtab_bench();
	this->gtab_about();
}

void GtkData::set_signals()
{
	/* Main window */

	/* Update Options::selected_page when user navigate to a different tab (required by do_refresh()) */
	this->notebook->signal_switch_page().connect([]([[maybe_unused]] Gtk::Widget *page, guint page_number)
	{
		Options::set_selected_page(static_cast<TabNumber>(page_number));
	});

	/* Show settings window */
	this->settingsbutton->signal_clicked().connect([this]
	{
		this->settingswindow->show();
	});

	/* Start daemon and reload CPU-X */
	this->daemonbutton->signal_clicked().connect([this]
	{
		this->daemonbutton->set_sensitive(false);
		const char *msg = start_daemon(true);
		if(msg == NULL)
		{
			this->data.reload = true;
			app->quit();
		}
		else
		{
			Gtk::MessageDialog dialog(*this->mainwindow, _(msg), false, Gtk::MessageType::MESSAGE_WARNING, Gtk::ButtonsType::BUTTONS_CLOSE);
			dialog.run();
			this->daemonbutton->set_sensitive(true);
		}
	});

	EXT_TAB_CPU(this->data.cpu)->activetype->signal_changed().connect(sigc::mem_fun(*this, &GtkData::change_active_type));
	EXT_TAB_CPU(this->data.cpu)->activecore->signal_changed().connect(sigc::mem_fun(*this, &GtkData::change_active_core));
	EXT_TAB_CACHES(this->data.caches)->activetest->signal_changed().connect(sigc::mem_fun(*this, &GtkData::change_active_test));
	EXT_TAB_MEMORY(this->data.memory)->activestick->signal_changed().connect(sigc::mem_fun(*this, &GtkData::change_active_stick));
	EXT_TAB_GRAPHICS(this->data.graphics)->activecard->signal_changed().connect(sigc::mem_fun(*this, &GtkData::change_active_card));
	EXT_LABEL_SWITCH(this->data.bench.prime_slow.state)->value->signal_state_set().connect(sigc::mem_fun(*this, &GtkData::start_bench_prime_slow), false);
	EXT_LABEL_SWITCH(this->data.bench.prime_fast.state)->value->signal_state_set().connect(sigc::mem_fun(*this, &GtkData::start_bench_prime_fast), false);
	EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.duration)->value->signal_value_changed().connect(sigc::mem_fun(*this, &GtkData::change_bench_duration));
	EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.threads)->value->signal_value_changed().connect(sigc::mem_fun(*this, &GtkData::change_bench_threads));
	this->refresh_handle = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &GtkData::grefresh), Options::get_refr_time());


	/* Settings window */

	/* Hide settings window and revert changes */
	this->settingswindow->signal_delete_event().connect([this]([[maybe_unused]] GdkEventAny *any_event)
	{
		settings->revert();
		this->settingswindow->hide();
		return true;
	});

	/* Event in settings window when CPU type is changed */
	this->defaulttype->signal_changed().connect([this]
	{
		this->fill_box_active_core(this->defaultcore);
	});

	/* Hide settings window and apply changes */
	this->validatebutton->signal_clicked().connect([this]
	{
		/* Apply new temperature unit */
		Options::set_temp_unit(static_cast<OptTempUnit>(settings->get_enum("temperature-unit")));

		/* Apply new refresh time */
		Options::set_refr_time(settings->get_uint("refresh-time"));
		this->refresh_handle.disconnect();
		this->refresh_handle = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &GtkData::grefresh), Options::get_refr_time());

		/* Apply new color theme */
		this->check_theme_color();
		this->set_colors();

		/* Save settings and close window */
		settings->apply();
		this->settingswindow->hide();
	});

	/* Hide settings window and revert changes */
	this->cancelbutton->signal_clicked().connect([this]
	{
		settings->revert();
		this->settingswindow->hide();

	});
}

void GtkData::bind_settings()
{
	settings->bind("temperature-unit",     this->temperatureunit,  "active-id");
	settings->bind("refresh-time",         this->refreshtime,      "value"    );
	settings->bind("gui-theme",            this->theme,            "active-id");
	settings->bind("default-tab",          this->defaulttab,       "active-id");
	settings->bind("default-core-type",    this->defaulttype,      "active"   );
	settings->bind("default-cpu-core",     this->defaultcore,      "active"   );
	settings->bind("default-cache-test",   this->defaultcachetest, "active"   );
	settings->bind("default-memory-stick", this->defaultstick,     "active"   );
	settings->bind("default-active-card",  this->defaultcard,      "active"   );
	settings->bind("print-cpuid-decimal",  this->cpuiddecimal,     "active"   );
	settings->bind("always-start-daemon",  this->startdaemon,      "active"   );
}

bool GtkData::grefresh()
{
	const auto page = Options::get_selected_page();

	switch(page)
	{
		case TAB_CPU:
			do_refresh(this->data, page);
			this->gtab_cpu();
			break;
		case TAB_CACHES:
			do_refresh(this->data, page);
			this->gtab_caches();
			break;
		case TAB_SYSTEM:
			do_refresh(this->data, page);
			this->gtab_system();
			break;
		case TAB_GRAPHICS:
			do_refresh(this->data, page);
			this->gtab_graphics();
			break;
		case TAB_BENCH:
			do_refresh(this->data, page);
			this->change_bench_sensitive(data.bench.is_running);
			this->gtab_bench();
			break;
		default:
			break;
	}

	return true;
}


/* CPU tab related methods */

GtkData::ExtTabCpu::ExtTabCpu(Glib::RefPtr<Gtk::Builder> builder) : ExtTab(builder, "cpulabel")
{
	builder->get_widget("proc_logocpu", this->logocpu);
	builder->get_widget("cpufooter_activetype", this->activetype);
	builder->get_widget("cpufooter_activecore", this->activecore);
}

void GtkData::gtab_cpu()
{
	auto& cpu_type = data.cpu.get_selected_cpu_type();

	set_tab_name(this->data.cpu);

	/* Processor frame */
	set_frame_name(cpu_type.processor);
	set_label_name_and_value(cpu_type.processor.vendor);
	set_label_name_and_value(cpu_type.processor.codename);
	set_label_name_and_value(cpu_type.processor.package);
	set_label_name_and_value(cpu_type.processor.technology);
	set_label_name_and_value(cpu_type.processor.voltage);
	set_label_name_and_value(cpu_type.processor.specification);
	set_label_name_and_value(cpu_type.processor.family, _("BaseFamily"));
	set_label_name_and_value(cpu_type.processor.dispfamily, _("CPU display (\"true\") family (computed as BaseFamily+ExtendedFamily)"));
	set_label_name_and_value(cpu_type.processor.model, _("BaseModel"));
	set_label_name_and_value(cpu_type.processor.dispmodel, _("CPU display (\"true\") model (computed as (ExtendedModel<<4)+BaseModel)"));
	set_label_name_and_value(cpu_type.processor.temperature);
	set_label_name_and_value(cpu_type.processor.stepping);
	set_label_name_and_value(cpu_type.processor.instructions);

	/* Clocks frame */
	set_frame_name(data.cpu.clocks);
	set_label_name_and_value(data.cpu.clocks.core_speed);
	set_label_name_and_value(data.cpu.clocks.multiplier);
	set_label_name_and_value(data.cpu.clocks.bus_speed);
	set_label_name_and_value(data.cpu.clocks.usage);

	/* Caches frame */
	set_frame_name(cpu_type.caches);
	set_label_name_and_value(cpu_type.caches.level1d);
	set_label_name_and_value(cpu_type.caches.level1i);
	set_label_name_and_value(cpu_type.caches.level2);
	set_label_name_and_value(cpu_type.caches.level3);

	/* Footer frame */
	set_label_name_and_value(cpu_type.footer.cores);
	set_label_name_and_value(cpu_type.footer.threads);
}

void GtkData::fill_box_active_type()
{
	fill_box_active_type(EXT_TAB_CPU(this->data.cpu)->activetype);
}

void GtkData::fill_box_active_type(Gtk::ComboBoxText *comboboxtext)
{
	if(this->data.cpu.cpu_types.size() > 0)
	{
		comboboxtext->remove_all();
		for(auto& type : this->data.cpu.cpu_types)
			comboboxtext->append(type.name);
		comboboxtext->set_active(Options::get_selected_type());
	}
	else
		comboboxtext->set_sensitive(false);
}

void GtkData::fill_box_active_core()
{
	fill_box_active_core(EXT_TAB_CPU(this->data.cpu)->activecore);
}

void GtkData::fill_box_active_core(Gtk::ComboBoxText *comboboxtext)
{
	auto& cpu_type = this->data.cpu.get_selected_cpu_type();

	if(cpu_type.footer.num_threads > 0)
	{
		comboboxtext->remove_all();
		for(auto i = 0; i < cpu_type.footer.num_threads; i++)
			comboboxtext->append(cpu_type.get_core_formatted(i));
		comboboxtext->set_active(Options::get_selected_core());
	}
	else
		comboboxtext->set_sensitive(false);
}

void GtkData::change_active_type()
{
	const int type = EXT_TAB_CPU(this->data.cpu)->activetype->get_active_row_number();
	if(type < 0)
		return;

	Options::set_selected_type(type, data.cpu.cpu_types.size());
	this->fill_box_active_core();
	this->grefresh();
}

void GtkData::change_active_core()
{
	const int core = EXT_TAB_CPU(this->data.cpu)->activecore->get_active_row_number();
	if(core < 0)
		return;

	Options::set_selected_core(core, data.cpu.get_selected_cpu_type().footer.num_threads);
}


/* Caches tab related methods */

GtkData::ExtTabCaches::ExtTabCaches(Glib::RefPtr<Gtk::Builder> builder) : ExtTab(builder, "cacheslabel")
{
	builder->get_widget("caches_grid", this->gridcaches);
	builder->get_widget("test_activetest", this->activetest);
}

void GtkData::gtab_caches()
{
	auto& cpu_type = data.caches.get_selected_cpu_type();
	static bool is_init = false;

	/* Hide tab if there is no cache */
	if(cpu_type.caches.size() == 0)
	{
		EXT_TAB_CACHES(this->data.caches)->gridcaches->hide();
		return;
	}

	set_tab_name(this->data.caches);

	/* Cache frame */
	for(auto& cache : cpu_type.caches)
	{
		set_frame_name(cache);
		set_label_name_and_value(cache.size);
		set_label_name_and_value(cache.speed);
	}

	if(!is_init)
	{
		is_init = true;

		/* Remove empty frames */
		for(int i = CacheLevels::L4; i >= int(cpu_type.caches.size()); i--)
			EXT_TAB_CACHES(this->data.caches)->gridcaches->remove_row(i);

		/* Test frame */
		set_frame_name(data.caches.test);
	}
}

void GtkData::fill_box_active_test()
{
	fill_box_active_test(EXT_TAB_CACHES(this->data.caches)->activetest);
}

void GtkData::fill_box_active_test(Gtk::ComboBoxText *comboboxtext)
{
	if(this->data.caches.test.names.size() > 0)
	{
		comboboxtext->remove_all();
		for(const auto& test_name : this->data.caches.test.names)
			comboboxtext->append(test_name);
		comboboxtext->set_active(Options::get_selected_test());
	}
	else
		comboboxtext->set_sensitive(false);
}

void GtkData::change_active_test()
{
	const int test = EXT_TAB_CACHES(this->data.caches)->activetest->get_active_row_number();
	if(test < 0)
		return;

	Options::set_selected_test(test);
}


/* Motherboard tab related methods */

GtkData::ExtTabMotherboard::ExtTabMotherboard(Glib::RefPtr<Gtk::Builder> builder) : ExtTab(builder, "motherboardlabel")
{
}

void GtkData::gtab_motherboard()
{
	set_tab_name(this->data.motherboard);

	/* Motherboard frame */
	set_frame_name(this->data.motherboard.board);
	set_label_name_and_value(this->data.motherboard.board.manufacturer);
	set_label_name_and_value(this->data.motherboard.board.model);
	set_label_name_and_value(this->data.motherboard.board.revision);

	/* BIOS frame */
	set_frame_name(this->data.motherboard.bios);
	set_label_name_and_value(this->data.motherboard.bios.brand);
	set_label_name_and_value(this->data.motherboard.bios.version);
	set_label_name_and_value(this->data.motherboard.bios.date);
	set_label_name_and_value(this->data.motherboard.bios.romsize);

	/* Chipset frame */
	set_frame_name(this->data.motherboard.chipset);
	set_label_name_and_value(this->data.motherboard.chipset.vendor);
	set_label_name_and_value(this->data.motherboard.chipset.model);
}


/* Memory tab related methods */

GtkData::ExtTabMemory::ExtTabMemory(Glib::RefPtr<Gtk::Builder> builder) : ExtTab(builder, "memorylabel")
{
	builder->get_widget("memory_scrolledwindow", this->scrolledsticks);
	builder->get_widget("sticks_activestick", this->activestick);
}

void GtkData::gtab_memory()
{
	/* Hide tab if there is no sticks */
	if(this->data.memory.sticks.size() == 0)
	{
		EXT_TAB_MEMORY(this->data.memory)->scrolledsticks->hide();
		return;
	}

	set_tab_name(this->data.memory);

	/* Stick frame */
	const auto& stick = data.memory.get_selected_stick();
	set_frame_name(stick);
	set_label_name_and_value(stick.manufacturer);
	set_label_name_and_value(stick.part_number);
	set_label_name_and_value(stick.type);
	set_label_name_and_value(stick.type_detail);
	set_label_name_and_value(stick.device_locator, _("Identify the physically-labeled socket or board position where the memory device is located"));
	set_label_name_and_value(stick.bank_locator, _("Identify the physically labeled bank where the memory device is located"));
	set_label_name_and_value(stick.size);
	set_label_name_and_value(stick.rank);
	set_label_name_and_value(stick.speed);
	set_label_name_and_value(stick.voltage);

	/* Sticks frame */
	set_frame_name(this->data.memory.footer);
}

void GtkData::fill_box_active_stick()
{
	fill_box_active_stick(EXT_TAB_MEMORY(this->data.memory)->activestick);
}

void GtkData::fill_box_active_stick(Gtk::ComboBoxText *comboboxtext)
{
	if(this->data.memory.sticks.size() > 0)
	{
		comboboxtext->remove_all();
		for(unsigned i = 0; i < this->data.memory.sticks.size(); i++)
			comboboxtext->append(this->data.memory.get_stick_formatted(i));
		comboboxtext->set_active(Options::get_selected_stick());
	}
	else
		comboboxtext->set_sensitive(false);
}

void GtkData::change_active_stick()
{
	const int stick = EXT_TAB_MEMORY(this->data.memory)->activestick->get_active_row_number();
	if(stick < 0)
		return;

	Options::set_selected_stick(stick, this->data.memory.sticks.size());
	this->gtab_memory();
}


/* System tab related methods */

GtkData::ExtTabSystem::ExtTabSystem(Glib::RefPtr<Gtk::Builder> builder) : ExtTab(builder, "systemlabel")
{
	builder->get_widget("mem_barused",  this->barused);
	builder->get_widget("mem_barbuff",  this->barbuff);
	builder->get_widget("mem_barcache", this->barcache);
	builder->get_widget("mem_barfree",  this->barfree);
	builder->get_widget("mem_barswap",  this->barswap);
}

void GtkData::gtab_system()
{
	set_tab_name(this->data.system);

	/* Operating System frame */
	set_frame_name(this->data.system.os);
	set_label_name_and_value(this->data.system.os.name);
	set_label_name_and_value(this->data.system.os.kernel);
	set_label_name_and_value(this->data.system.os.hostname);
	set_label_name_and_value(this->data.system.os.uptime);

	/* Memory frame */
	set_frame_name(this->data.system.memory);
	set_label_name_and_value(this->data.system.memory.used);
	set_label_name_and_value(this->data.system.memory.buffers);
	set_label_name_and_value(this->data.system.memory.cached);
	set_label_name_and_value(this->data.system.memory.free);
	set_label_name_and_value(this->data.system.memory.swap);
}

bool GtkData::draw_bar_memory_used(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if(this->data.system.memory.mem_total == 0)
		return false;

	double before = 0;
	double percent = this->data.system.memory.mem_used / this->data.system.memory.mem_total * 100;

	if(this->is_dark_theme)
		cr->set_source_rgba(0.01, 0.02, 0.37, 1.00);
	else
		cr->set_source_rgba(1.00, 0.92, 0.23, 1.00);

	return draw_bar_memory(cr, EXT_TAB_SYSTEM(this->data.system)->barused, EXT_LABEL(this->data.system.memory.used)->value, before, percent);
}

bool GtkData::draw_bar_memory_buffers(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if(this->data.system.memory.mem_total == 0)
		return false;

	double before  = this->data.system.memory.mem_used    / this->data.system.memory.mem_total * 100;
	double percent = this->data.system.memory.mem_buffers / this->data.system.memory.mem_total * 100;

	if(this->is_dark_theme)
		cr->set_source_rgba(0.01, 0.24, 0.54, 1.00);
	else
		cr->set_source_rgba(0.13, 0.59, 0.95, 1.00);

	return draw_bar_memory(cr, EXT_TAB_SYSTEM(this->data.system)->barbuff, EXT_LABEL(this->data.system.memory.buffers)->value, before, percent);
}

bool GtkData::draw_bar_memory_cached(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if(this->data.system.memory.mem_total == 0)
		return false;

	double before  = this->data.system.memory.mem_used    / this->data.system.memory.mem_total * 100;
	before        += this->data.system.memory.mem_buffers / this->data.system.memory.mem_total * 100;
	double percent = this->data.system.memory.mem_cached  / this->data.system.memory.mem_total * 100;

	if(this->is_dark_theme)
		cr->set_source_rgba(0.00, 0.47, 0.71, 1.00) ;
	else
		cr->set_source_rgba(0.61, 0.15, 0.69, 1.00);

	return draw_bar_memory(cr, EXT_TAB_SYSTEM(this->data.system)->barcache, EXT_LABEL(this->data.system.memory.cached)->value, before, percent);
}

bool GtkData::draw_bar_memory_free(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if(this->data.system.memory.mem_total == 0)
		return false;

	double before  = this->data.system.memory.mem_used    / this->data.system.memory.mem_total * 100;
	before        += this->data.system.memory.mem_buffers / this->data.system.memory.mem_total * 100;
	before        += this->data.system.memory.mem_cached  / this->data.system.memory.mem_total * 100;
	double percent = this->data.system.memory.mem_free    / this->data.system.memory.mem_total * 100;

	if(this->is_dark_theme)
		cr->set_source_rgba(0.00, 0.59, 0.78, 1.00);
	else
		cr->set_source_rgba(0.30, 0.69, 0.31, 1.00);

	return draw_bar_memory(cr, EXT_TAB_SYSTEM(this->data.system)->barfree, EXT_LABEL(this->data.system.memory.free)->value, before, percent);
}

bool GtkData::draw_bar_memory_swap(const Cairo::RefPtr<Cairo::Context> &cr)
{
	if(this->data.system.memory.swap_total == 0)
		return false;

	double before = 0;
	double percent = this->data.system.memory.swap_used / this->data.system.memory.swap_total * 100;

	if(this->is_dark_theme)
		cr->set_source_rgba(0.00, 0.71, 0.85, 1.00);
	else
		cr->set_source_rgba(0.96, 0.26, 0.21, 1.00);

	return draw_bar_memory(cr, EXT_TAB_SYSTEM(this->data.system)->barswap, EXT_LABEL(this->data.system.memory.swap)->value, before, percent);
}

bool GtkData::draw_bar_memory(const Cairo::RefPtr<Cairo::Context> &cr, Gtk::DrawingArea *area, Gtk::Label *label, double before, double percent)
{
	if(std::isnan(percent))
		return false;

	auto newlayout    = label->get_layout()->copy();
	const auto width  = area->get_allocated_width();
	const auto height = area->get_allocated_height();

	/* Print a colored rectangle */
	cr->rectangle(before / 100 * width, 0, percent / 100 * width, height);
	cr->fill();

	/* Print percentage */
	if(this->is_dark_theme)
		cr->set_source_rgb(192.0 / 255.0, 192.0 / 255.0, 0.0); // yellow text for dark theme
	else
		cr->set_source_rgb(0.0, 0.0, 128.0 / 255.0); // blue text for light theme
	cr->move_to(-40, 0);
	newlayout->set_text(string_format("%.2f%%", percent));
	newlayout->show_in_cairo_context(cr);
	cr->fill();

	return true;
}


/* Graphics tab related methods */

GtkData::ExtTabGraphics::ExtTabGraphics(Glib::RefPtr<Gtk::Builder> builder) : ExtTab(builder, "graphicslabel")
{
	builder->get_widget("graphics_scrolledwindow", this->scrolledcards);
	builder->get_widget("cards_activecard", this->activecard);
}

void GtkData::gtab_graphics()
{
	/* Hide tab if there is no cards */
	if(this->data.graphics.cards.size() == 0)
	{
		EXT_TAB_GRAPHICS(this->data.graphics)->scrolledcards->hide();
		return;
	}

	set_tab_name(this->data.graphics);

	/* Card frame */
	const auto& card = data.graphics.get_selected_card();
	set_frame_name(card);
	set_label_name_and_value(card.vendor);
	set_label_name_and_value(card.kernel_driver);
	set_label_name_and_value(card.user_mode_driver, _("User Mode Driver Version"));
	set_label_name_and_value(card.model);
	set_label_name_and_value(card.comp_unit);
	set_label_name_and_value(card.device_id);
	set_label_name_and_value(card.vbios_version);
	set_label_name_and_value(card.interface);
	set_label_name_and_value(card.temperature);
	set_label_name_and_value(card.usage);
	set_label_name_and_value(card.core_voltage);
	set_label_name_and_value(card.power_avg);
	set_label_name_and_value(card.core_clock);
	set_label_name_and_value(card.mem_clock);
	set_label_name_and_value(card.mem_used);
	set_label_name_and_value(card.resizable_bar);
	set_label_name_and_value(card.vulkan_rt, _("Vulkan Ray Tracing"));
	set_label_name_and_value(card.opengl_version);
	set_label_name_and_value(card.vulkan_version);
	set_label_name_and_value(card.opencl_version);

	/* Cards frame */
	set_frame_name(this->data.graphics.footer);
}

void GtkData::fill_box_active_card()
{
	fill_box_active_card(EXT_TAB_GRAPHICS(this->data.graphics)->activecard);
}

void GtkData::fill_box_active_card(Gtk::ComboBoxText *comboboxtext)
{
	if(this->data.graphics.cards.size() > 0)
	{
		comboboxtext->remove_all();
		for(unsigned i = 0; i < this->data.graphics.cards.size(); i++)
			comboboxtext->append(this->data.graphics.get_card_formatted(i));
		comboboxtext->set_active(Options::get_selected_gpu());
	}
	else
		comboboxtext->set_sensitive(false);
}

void GtkData::change_active_card()
{
	const int card = EXT_TAB_GRAPHICS(this->data.graphics)->activecard->get_active_row_number();
	if(card < 0)
		return;

	Options::set_selected_type(card, this->data.graphics.cards.size());
	this->grefresh();
}


/* Bench tab related methods */

GtkData::ExtTabBench::ExtTabBench(Glib::RefPtr<Gtk::Builder> builder) : ExtTab(builder, "benchlabel")
{
}

void GtkData::gtab_bench()
{
	set_tab_name(this->data.bench);

	/* Prime Slow frame */
	set_frame_name(this->data.bench.prime_slow);
	EXT_LABEL(this->data.bench.prime_slow.score)->name->set_text(this->data.bench.prime_slow.score.name);
	EXT_LABEL_PROGRESS_BAR(this->data.bench.prime_slow.score)->value->set_text(this->data.bench.prime_slow.score.value);
	EXT_LABEL(this->data.bench.prime_slow.state)->name->set_text(this->data.bench.prime_slow.state.name);

	/* Prime Fast frame */
	set_frame_name(this->data.bench.prime_fast);
	EXT_LABEL(this->data.bench.prime_fast.score)->name->set_text(this->data.bench.prime_fast.score.name);
	EXT_LABEL_PROGRESS_BAR(this->data.bench.prime_fast.score)->value->set_text(this->data.bench.prime_fast.score.value);
	EXT_LABEL(this->data.bench.prime_fast.state)->name->set_text(this->data.bench.prime_fast.state.name);

	/* Parameters frame */
	set_frame_name(this->data.bench.parameters);
	EXT_LABEL(this->data.bench.parameters.duration)->name->set_text(this->data.bench.parameters.duration.name);
	EXT_LABEL(this->data.bench.parameters.threads)->name->set_text(this->data.bench.parameters.threads.name);
}

bool GtkData::start_bench_prime_slow(bool state)
{
	if(state)
	{
		this->data.bench.fast_mode = false;
		start_benchmarks(data);
	}
	else
		data.bench.is_running = false;
	grefresh();

	return false;
}

bool GtkData::start_bench_prime_fast(bool state)
{
	if(state)
	{
		this->data.bench.fast_mode = true;
		start_benchmarks(data);
	}
	else
		data.bench.is_running = false;
	grefresh();

	return false;
}

void GtkData::change_bench_sensitive(bool state)
{
	auto *gtk_switch_active  = this->data.bench.fast_mode ? EXT_LABEL_SWITCH(this->data.bench.prime_fast.state)->value : EXT_LABEL_SWITCH(this->data.bench.prime_slow.state)->value;
	auto *gtk_switch_standby = this->data.bench.fast_mode ? EXT_LABEL_SWITCH(this->data.bench.prime_slow.state)->value : EXT_LABEL_SWITCH(this->data.bench.prime_fast.state)->value;
	auto *gtk_progress_bar   = this->data.bench.fast_mode ? EXT_LABEL_PROGRESS_BAR(this->data.bench.prime_fast.score)->value : EXT_LABEL_PROGRESS_BAR(this->data.bench.prime_slow.score)->value;

	if(state)
	{
		gtk_switch_standby->set_sensitive(false);
		EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.threads)->value->set_sensitive(false);
		gtk_progress_bar->set_fraction(this->data.bench.parameters.elapsed_i / double(this->data.bench.parameters.duration_i * 60));
	}
	else
	{
		gtk_switch_active->set_active(false);
		gtk_switch_standby->set_sensitive(true);
		EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.threads)->value->set_sensitive(true);
		if(this->data.bench.is_completed)
			gtk_progress_bar->set_fraction(1.0);
	}
}

void GtkData::change_bench_duration()
{
	const int duration = EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.duration)->value->get_value_as_int();
	if(duration < 0)
		return;

	data.bench.parameters.set_duration(duration);
}

void GtkData::change_bench_threads()
{
	const int threads = EXT_LABEL_SPIN_BUTTON(this->data.bench.parameters.threads)->value->get_value_as_int();

	if(threads < 0)
		return;

	data.bench.parameters.set_threads(threads);
}


/* About tab related methods */

GtkData::ExtTabAbout::ExtTabAbout(Glib::RefPtr<Gtk::Builder> builder) : ExtTab(builder, "aboutlabel")
{
	builder->get_widget("about_logoprg", this->logoprg);
	builder->get_widget("about_descr", this->about_descr);
	builder->get_widget("about_version", this->about_version);
	builder->get_widget("about_author", this->about_author);
	builder->get_widget("about_website", this->about_website);
	builder->get_widget("license_copyright", this->license_copyright);
	builder->get_widget("license_name", this->license_name);
	builder->get_widget("license_warranty", this->license_warranty);
}

void GtkData::gtab_about()
{
	set_tab_name(this->data.about);

	/* Header frame */
	EXT_TAB_ABOUT(this->data.about)->about_descr->set_text(this->data.about.description.text);

	/* About frame */
	set_frame_name(this->data.about.about);
	EXT_TAB_ABOUT(this->data.about)->about_version->set_text(this->data.about.about.version);
	EXT_TAB_ABOUT(this->data.about)->about_author->set_text(this->data.about.about.author);
	EXT_TAB_ABOUT(this->data.about)->about_website->set_text(this->data.about.about.website);

	/* License frame */
	set_frame_name(this->data.about.license);
	EXT_TAB_ABOUT(this->data.about)->license_copyright->set_text(this->data.about.license.copyright);
	EXT_TAB_ABOUT(this->data.about)->license_name->set_text(this->data.about.license.name);
	EXT_TAB_ABOUT(this->data.about)->license_warranty->set_text(this->data.about.license.warranty);
}


/************************* Public function *************************/

/* Load and apply settings from GSettings */
void load_settings()
{
	Glib::init();
	settings = Gio::Settings::create(APPLICATION_ID);
	settings->delay();
	Options::set_temp_unit     (static_cast<OptTempUnit>(settings->get_enum("temperature-unit")));
	Options::set_refr_time     (settings->get_uint("refresh-time"));
	Options::set_selected_page (static_cast<TabNumber>(settings->get_enum("default-tab")));
	Options::set_selected_type (settings->get_uint("default-core-type"), -1);
	Options::set_selected_core (settings->get_uint("default-cpu-core"), -1);
	Options::set_selected_test (settings->get_uint("default-cache-test"));
	Options::set_selected_stick(settings->get_uint("default-memory-stick"), -1);
	Options::set_selected_gpu  (settings->get_uint("default-active-card"), -1);
	Options::set_cpuid_decimal (settings->get_boolean("print-cpuid-decimal"));
	Options::set_with_daemon   (settings->get_boolean("always-start-daemon"));
}

/* Start CPU-X in GTK mode */
int start_gui_gtk(Data &data)
{
	MSG_VERBOSE("%s", _("Starting GTK GUI…"));
	try
	{
		app                = Gtk::Application::create(APPLICATION_ID, Gio::APPLICATION_NON_UNIQUE);
		const auto builder = Gtk::Builder::create_from_file(get_data_path("cpu-x-gtk-3.12.ui"));
		Glib::set_prgname(APPLICATION_ID);
		GtkData gdata(builder, data);
		return app->run(*gdata.mainwindow);
	}
	catch(const Gtk::BuilderError& e)
	{
		std::cerr << e.what() << std::endl;
		MSG_ERROR("%s", _("failed to import UI in GtkBuilder"));
		return EXIT_FAILURE;
	}
}
