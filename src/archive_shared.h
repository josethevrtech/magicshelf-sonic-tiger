#pragma once
#include <atomic>
#include <cstdint>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
constexpr int ArchiveWidth=758,ArchiveHeight=540;
struct ArchiveShared {
 std::atomic<uint32_t> frameSequence{0},frames{0},status{0},keys{0},reload{0},gain{28},audible{0},quit{0};
 std::atomic<uint32_t> deliveredKeys{0},deliveredEvents{0};
 uint8_t pixels[ArchiveWidth*ArchiveHeight*4]{};
};
struct ArchiveMapping {
 int fd=-1;ArchiveShared* shared=nullptr;
 ArchiveMapping(const std::string& path,bool create){
  fd=::open(path.c_str(),O_RDWR|O_CLOEXEC|(create?O_CREAT:0),0600);
  if(fd<0)throw std::runtime_error("Cannot open player buffer");
  if(create&&ftruncate(fd,sizeof(ArchiveShared)))throw std::runtime_error("Cannot size player buffer");
  struct stat st{};if(fstat(fd,&st)||st.st_size!=sizeof(ArchiveShared))throw std::runtime_error("Invalid player buffer");
  auto p=mmap(nullptr,sizeof(ArchiveShared),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
  if(p==MAP_FAILED)throw std::runtime_error("Cannot map player buffer");shared=static_cast<ArchiveShared*>(p);
  if(create)new(shared)ArchiveShared();
 }
 ~ArchiveMapping(){if(shared)munmap(shared,sizeof(ArchiveShared));if(fd>=0)close(fd);}
};
