if not get_option('dbus-backend').enabled()
	conf_data.set10('WLR_HAS_DBUS_BACKEND', false)
	subdir_done()
endif

wlr_files += files(
	'backend.c',
	'event.c',
	'pointer.c',
	'keyboard.c',
	'asci2scan.c',
)
conf_data.set10('WLR_HAS_DBUS_BACKEND', true)
