add_library(classicui MODULE classicui.cpp)
target_link_libraries(classicui Fcitx5::Core Cairo::Cairo Cairo::XCB Cairo::EGL)
set_target_properties(classicui PROPERTIES PREFIX "")
install(TARGETS classicui DESTINATION "${FCITX_INSTALL_ADDONDIR}")
install(FILES classicui.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
