// Begin CVS Header
//   $Source: /fs/turing/cvs/copasi_dev/copasi/commandline/COptions.cpp,v $
//   $Revision: 1.42 $
//   $Name: Build-33 $
//   $Author: shoops $
//   $Date: 2010/08/02 16:46:19 $
// End CVS Header

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <string.h>
#include <stdlib.h>

#include "copasi.h"

#ifdef WIN32
# include <windows.h>
# include <winbase.h>
# include <direct.h>
# ifdef ERROR
#  undef ERROR
# endif
#else
# include <unistd.h>
#endif

#ifdef Darwin
# include <Carbon/Carbon.h>
#endif

#include <sstream>
#include <errno.h>

#include "COptionParser.h"
#include "COptions.h"

#include "utilities/CCopasiMessage.h"
#include "utilities/CDirEntry.h"
#include "utilities/utility.h"

COptions::optionType COptions::mOptions;
COptions::nonOptionType COptions::mNonOptions;

COptions::COptions()
{CONSTRUCTOR_TRACE;}

COptions::~COptions()
{DESTRUCTOR_TRACE;}

void COptions::init(C_INT argc, char *argv[])
{
  char **ArgV = new char * [argc];
  C_INT ArgC = 0;

  if (argc > 0)
    setValue("Self", localeToUtf8(argv[0]));
  else
    setValue("Self", std::string(""));

  setValue("PWD", getPWD());

  // First we must clean up the command line by
  // taking out any SBW commands like -sbwregister and -sbwmodule

  // The default settings for SBW related options
  setValue("SBWRegister", false);
  setValue("SBWModule", false);

  C_INT i;

  for (i = 0; i < argc; i++)
    {
      if (strcmp(argv[i], "-sbwregister") == 0)
        setValue("SBWRegister", true);
      else if (strcmp(argv[i], "-sbwmodule") == 0)
        setValue("SBWModule", true);
      else
        {
          ArgV[ArgC] = argv[i];
          ArgC++;
        }
    }

  // Now we are ready to start the Clo++ generated parser.
  copasi::COptionParser * pPreParser = new copasi::COptionParser;
  pPreParser->parse(ArgC, ArgV);

  const copasi::options &PreOptions = pPreParser->get_options();

  setValue("CopasiDir", localeToUtf8(PreOptions.CopasiDir));

  if (compareValue("CopasiDir", (std::string) ""))
    {
      setValue("CopasiDir", getCopasiDir());
    }

  setValue("Home", localeToUtf8(PreOptions.Home));

  if (compareValue("Home", (std::string) ""))
    setValue("Home", getHome());

  setValue("Tmp", localeToUtf8(PreOptions.Tmp));

  if (compareValue("Tmp", (std::string) ""))
    setValue("Tmp", getTemp());

  setValue("ConfigDir", localeToUtf8(PreOptions.ConfigDir));

  if (compareValue("ConfigDir", (std::string) ""))
    setValue("ConfigDir", getConfigDir());

  setValue("ConfigFile", localeToUtf8(PreOptions.ConfigFile));

  if (compareValue("ConfigFile", (std::string) ""))
    setValue("ConfigFile", getConfigFile());

  mNonOptions.clear();
  std::vector< std::string >::const_iterator it = pPreParser->get_non_options().begin();
  std::vector< std::string >::const_iterator end = pPreParser->get_non_options().end();

  for (; it != end; ++it)
    mNonOptions.push_back(localeToUtf8(*it));

  const copasi::options &Options = pPreParser->get_options();

  std::string CopasiDir;
  getValue("CopasiDir", CopasiDir);
  std::string Home;
  getValue("Home", Home);

  /* The values for ExampleDir and WizardDir are dependent on CopasiDir
     and on the OS. */

#ifdef Darwin
  setValue("DefaultConfigDir", CDirEntry::dirName(CopasiDir) + "/config");
  setValue("ExampleDir", CDirEntry::dirName(CopasiDir) + "/examples");
  setValue("WizardDir", CopasiDir + "/Contents/Resources/doc/html");
#elif WIN32
  setValue("DefaultConfigDir", CopasiDir + "\\share\\copasi\\config");
  setValue("ExampleDir", CopasiDir + "\\share\\copasi\\examples");
  setValue("WizardDir", CopasiDir + "\\share\\copasi\\doc\\html");
#else // All Unix flavors have the same installation structure.
  setValue("DefaultConfigDir", CopasiDir + "/share/copasi/config");
  setValue("ExampleDir", CopasiDir + "/share/copasi/examples");
  setValue("WizardDir", CopasiDir + "/share/copasi/doc/html");
#endif

  /* Create manually for each option except for:
     CopasiDir, ConfigFile, Home, and Default
     setValue("OptionId", Options.OptionID); */

  if (Options.Tmp != "") setValue("Tmp", Options.Tmp);

  setValue("NoLogo", Options.NoLogo);
  setValue("Validate", Options.Validate);
  setValue("Verbose", Options.Verbose);
  setValue("License", Options.License);
  setValue("Save", localeToUtf8(Options.Save));
  setValue("ImportSBML", localeToUtf8(Options.ImportSBML));
  setValue("ExportSBML", localeToUtf8(Options.ExportSBML));
  setValue("SBMLSchema", Options.SBMLSchema);
  setValue("ExportC", localeToUtf8(Options.ExportC));
  setValue("ExportXPPAUT", localeToUtf8(Options.ExportXPPAUT));
  setValue("ExportBerkeleyMadonna", localeToUtf8(Options.ExportBerkeleyMadonna));


  delete pPreParser;
  delete [] ArgV;
}

void COptions::cleanup()
{
  optionType::iterator begin = mOptions.begin();
  optionType::iterator end = mOptions.end();

  for (; begin != end; begin++) pdelete(begin->second);
}

const COptions::nonOptionType & COptions::getNonOptions() {return mNonOptions;}

std::string COptions::getEnvironmentVariable(const std::string & name)
{
  char * value = getenv(name.c_str());

  if (value) return localeToUtf8(value);
  else return "";
}

std::string COptions::getCopasiDir(void)
{
  std::string CopasiDir;

  CopasiDir = getEnvironmentVariable("COPASIDIR");

#ifdef WIN32

  if (CopasiDir == "")
    {
      size_t PrgNameSize = 256;
      size_t Returned;
      char * PrgName = new char[PrgNameSize];

      while (!(Returned = GetModuleFileName(NULL, PrgName, PrgNameSize)) ||
             PrgNameSize == Returned)
        {
          if (GetLastError() != ERROR_ALREADY_EXISTS)
            {
              *PrgName = '\0';
              break;
            }

          delete [] PrgName;
          PrgNameSize *= 2;
          PrgName = new char[PrgNameSize];
        }

      CopasiDir = PrgName;
      delete [] PrgName;

      /* Get rid of the executable */
      CopasiDir = CDirEntry::dirName(CopasiDir);

      /* Get rid of bin or sbin */
      CopasiDir = localeToUtf8(CDirEntry::dirName(CopasiDir));
    }

#endif // WIN32

#ifdef Darwin

  if (CopasiDir == "")
    {
      CFBundleRef MainBundleRef = NULL;
      MainBundleRef = CFBundleGetMainBundle();

      if (MainBundleRef != NULL)
        {
          CFURLRef pluginRef = NULL;
          pluginRef = CFBundleCopyBundleURL(MainBundleRef);

          if (pluginRef != NULL)
            {
              CFStringRef macPath = NULL;
              macPath = CFURLCopyFileSystemPath(pluginRef, kCFURLPOSIXPathStyle);

              if (macPath != NULL)
                {
                  CFIndex size = CFStringGetLength(macPath);
                  char* cString = new char[size + 1];
                  CFStringGetCString(macPath, cString, size + 1, kCFStringEncodingUTF8);
                  CopasiDir = localeToUtf8(cString);
                  delete[] cString;
                }
            }
        }
    }

#endif // Darwin

  if (CopasiDir == "")
    CCopasiMessage Message(CCopasiMessage::RAW, MCConfiguration + 1);

  return CopasiDir;
}

std::string COptions::getPWD(void)
{
  size_t PWDSize = 256;
  char * PWD = NULL;

  while (!(PWD = getcwd(NULL, PWDSize)))
    {
      if (errno != ERANGE) break;

      PWDSize *= 2;
    }

  std::string pwd;

  if (PWD)
    {
      pwd = PWD;
      free(PWD);
    }
  else
    pwd = "";

  return localeToUtf8(pwd);
}

std::string COptions::getHome(void)
{
  std::string Home;

  Home = getEnvironmentVariable("HOME");

#ifdef WIN32

  if (Home == "")
    Home = getEnvironmentVariable("HOMEDRIVE")
           + getEnvironmentVariable("HOMEPATH");

#endif // WIN32

  if (Home == "")
    {
      std::ostringstream error;
      error << std::endl
      << "  use --home HOME" << std::endl
      << "  or set the environment variable HOME" << std::endl
      << "  to point to your home directory" << std::endl;

      throw copasi::option_error(error.str());
    }

  return Home;
}

std::string COptions::getTemp(void)
{
  std::string Temp, User, CreateCopasiDir, CreateUserDir;

  Temp = getEnvironmentVariable("TEMP");

  if (Temp == "") Temp = getEnvironmentVariable("TMP");

  User = getEnvironmentVariable("USER");

  if (User == "") User = getEnvironmentVariable("USERNAME");

  if (User == "") User = "CopasiUser";

  if (Temp == "") // OS specific fallback.
#ifdef WIN32
    Temp = getEnvironmentVariable("windir") + CDirEntry::Separator + "Temp";

#else
    Temp = "/tmp";
#endif // WIN32

  // Assure that Temp exists and is a directory.
  if (!CDirEntry::isDir(Temp) || !CDirEntry::isWritable(Temp))
    return "";

  CreateCopasiDir = Temp + CDirEntry::Separator + "copasi";
  CreateUserDir = CreateCopasiDir + CDirEntry::Separator + User;

  //Assure that CreateCopasiDir exists and is a writable directory.
  if (!CDirEntry::createDir("copasi", Temp))
    return Temp;

  Temp = CreateCopasiDir;

  //Assure that CreateUserDir exists and is a writable directory.
  if (!CDirEntry::createDir(User, Temp))
    return Temp;

  Temp = CreateUserDir;
  return Temp;
}

std::string COptions::getConfigDir(void)
{
  std::string Home;

  getValue("Home", Home);

  if (!CDirEntry::createDir(".copasi", Home))
    return Home;

  return Home + CDirEntry::Separator + ".copasi";
}

std::string COptions::getConfigFile(void)
{
  std::string ConfigDir;

  getValue("ConfigDir", ConfigDir);

  return ConfigDir + CDirEntry::Separator + "copasi";
}
