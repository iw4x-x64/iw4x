#include <filesystem>

using namespace std::filesystem;

namespace iw4x
{
  bool build_installed = false;

  // Base installation directories.
  //

#ifdef LIBIW4X_INSTALL_ROOT
  path build_install_root (LIBIW4X_INSTALL_ROOT);
#else
  path build_install_root;
#endif

#ifdef LIBIW4X_INSTALL_ROOT_RELATIVE
  path build_install_root_relative (LIBIW4X_INSTALL_ROOT_RELATIVE);
#else
  path build_install_root_relative;
#endif

#ifdef LIBIW4X_INSTALL_DATA_ROOT
  path build_install_data_root (LIBIW4X_INSTALL_DATA_ROOT);
#else
  path build_install_data_root;
#endif

#ifdef LIBIW4X_INSTALL_DATA_ROOT_RELATIVE
  path build_install_data_root_relative (LIBIW4X_INSTALL_DATA_ROOT_RELATIVE);
#else
  path build_install_data_root_relative;
#endif

#ifdef LIBIW4X_INSTALL_EXEC_ROOT
  path build_install_exec_root (LIBIW4X_INSTALL_EXEC_ROOT);
#else
  path build_install_exec_root;
#endif

#ifdef LIBIW4X_INSTALL_EXEC_ROOT_RELATIVE
  path build_install_exec_root_relative (LIBIW4X_INSTALL_EXEC_ROOT_RELATIVE);
#else
  path build_install_exec_root_relative;
#endif

  // Executables and libraries.
  //

#ifdef LIBIW4X_INSTALL_BIN
  path build_install_bin (LIBIW4X_INSTALL_BIN);
#else
  path build_install_bin;
#endif

#ifdef LIBIW4X_INSTALL_BIN_RELATIVE
  path build_install_bin_relative (LIBIW4X_INSTALL_BIN_RELATIVE);
#else
  path build_install_bin_relative;
#endif

#ifdef LIBIW4X_INSTALL_SBIN
  path build_install_sbin (LIBIW4X_INSTALL_SBIN);
#else
  path build_install_sbin;
#endif

#ifdef LIBIW4X_INSTALL_SBIN_RELATIVE
  path build_install_sbin_relative (LIBIW4X_INSTALL_SBIN_RELATIVE);
#else
  path build_install_sbin_relative;
#endif

#ifdef LIBIW4X_INSTALL_LIB
  path build_install_lib (LIBIW4X_INSTALL_LIB);
#else
  path build_install_lib;
#endif

#ifdef LIBIW4X_INSTALL_LIB_RELATIVE
  path build_install_lib_relative (LIBIW4X_INSTALL_LIB_RELATIVE);
#else
  path build_install_lib_relative;
#endif

#ifdef LIBIW4X_INSTALL_LIBEXEC
  path build_install_libexec (LIBIW4X_INSTALL_LIBEXEC);
#else
  path build_install_libexec;
#endif

#ifdef LIBIW4X_INSTALL_LIBEXEC_RELATIVE
  path build_install_libexec_relative (LIBIW4X_INSTALL_LIBEXEC_RELATIVE);
#else
  path build_install_libexec_relative;
#endif

#ifdef LIBIW4X_INSTALL_PKGCONFIG
  path build_install_pkgconfig (LIBIW4X_INSTALL_PKGCONFIG);
#else
  path build_install_pkgconfig;
#endif

#ifdef LIBIW4X_INSTALL_PKGCONFIG_RELATIVE
  path build_install_pkgconfig_relative (LIBIW4X_INSTALL_PKGCONFIG_RELATIVE);
#else
  path build_install_pkgconfig_relative;
#endif

  // Architecture-independent data and configuration.
  //

#ifdef LIBIW4X_INSTALL_ETC
  path build_install_etc (LIBIW4X_INSTALL_ETC);
#else
  path build_install_etc;
#endif

#ifdef LIBIW4X_INSTALL_ETC_RELATIVE
  path build_install_etc_relative (LIBIW4X_INSTALL_ETC_RELATIVE);
#else
  path build_install_etc_relative;
#endif

#ifdef LIBIW4X_INSTALL_INCLUDE
  path build_install_include (LIBIW4X_INSTALL_INCLUDE);
#else
  path build_install_include;
#endif

#ifdef LIBIW4X_INSTALL_INCLUDE_RELATIVE
  path build_install_include_relative (LIBIW4X_INSTALL_INCLUDE_RELATIVE);
#else
  path build_install_include_relative;
#endif

#ifdef LIBIW4X_INSTALL_INCLUDE_ARCH
  path build_install_include_arch (LIBIW4X_INSTALL_INCLUDE_ARCH);
#else
  path build_install_include_arch;
#endif

#ifdef LIBIW4X_INSTALL_INCLUDE_ARCH_RELATIVE
  path build_install_include_arch_relative (LIBIW4X_INSTALL_INCLUDE_ARCH_RELATIVE);
#else
  path build_install_include_arch_relative;
#endif

#ifdef LIBIW4X_INSTALL_SHARE
  path build_install_share (LIBIW4X_INSTALL_SHARE);
#else
  path build_install_share;
#endif

#ifdef LIBIW4X_INSTALL_SHARE_RELATIVE
  path build_install_share_relative (LIBIW4X_INSTALL_SHARE_RELATIVE);
#else
  path build_install_share_relative;
#endif

#ifdef LIBIW4X_INSTALL_DATA
  path build_install_data (LIBIW4X_INSTALL_DATA);
#else
  path build_install_data;
#endif

#ifdef LIBIW4X_INSTALL_DATA_RELATIVE
  path build_install_data_relative (LIBIW4X_INSTALL_DATA_RELATIVE);
#else
  path build_install_data_relative;
#endif

#ifdef LIBIW4X_INSTALL_BUILDFILE
  path build_install_buildfile (LIBIW4X_INSTALL_BUILDFILE);
#else
  path build_install_buildfile;
#endif

#ifdef LIBIW4X_INSTALL_BUILDFILE_RELATIVE
  path build_install_buildfile_relative (LIBIW4X_INSTALL_BUILDFILE_RELATIVE);
#else
  path build_install_buildfile_relative;
#endif

  // Documentation and man pages.
  //

#ifdef LIBIW4X_INSTALL_DOC
  path build_install_doc (LIBIW4X_INSTALL_DOC);
#else
  path build_install_doc;
#endif

#ifdef LIBIW4X_INSTALL_DOC_RELATIVE
  path build_install_doc_relative (LIBIW4X_INSTALL_DOC_RELATIVE);
#else
  path build_install_doc_relative;
#endif

#ifdef LIBIW4X_INSTALL_LEGAL
  path build_install_legal (LIBIW4X_INSTALL_LEGAL);
#else
  path build_install_legal;
#endif

#ifdef LIBIW4X_INSTALL_LEGAL_RELATIVE
  path build_install_legal_relative (LIBIW4X_INSTALL_LEGAL_RELATIVE);
#else
  path build_install_legal_relative;
#endif

#ifdef LIBIW4X_INSTALL_MAN
  path build_install_man (LIBIW4X_INSTALL_MAN);
#else
  path build_install_man;
#endif

#ifdef LIBIW4X_INSTALL_MAN_RELATIVE
  path build_install_man_relative (LIBIW4X_INSTALL_MAN_RELATIVE);
#else
  path build_install_man_relative;
#endif
}
