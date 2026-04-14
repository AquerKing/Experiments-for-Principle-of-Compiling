#include "CommandLine.h"
#include "ProgramRuntimeInfo.h"

ProgramRuntimeInfo RuntimeInfo;

int main(int argc, char *argv[]) {
  HandleCommandLine(argc, argv);
  return 0;
}