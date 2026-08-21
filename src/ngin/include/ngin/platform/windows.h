#pragma once
#include <Windows.h>

#include <atomic>
#include <cstdio>
#include <filesystem>
#include <thread>

namespace Ngin {
inline std::filesystem::path getExecutableDirectory() {
  wchar_t buffer[MAX_PATH];
  GetModuleFileNameW(nullptr, buffer, MAX_PATH);

  return std::filesystem::path(buffer).parent_path();
}

class DebugOutputToStdout {
 public:
  DebugOutputToStdout() {
    // DBWIN objects used by OutputDebugString.
    m_bufferReady = CreateEventW(nullptr, FALSE, FALSE, L"DBWIN_BUFFER_READY");

    m_dataReady = CreateEventW(nullptr, FALSE, FALSE, L"DBWIN_DATA_READY");

    m_mapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 4096,
        L"DBWIN_BUFFER");

    if (!m_bufferReady || !m_dataReady || !m_mapping)
      return;

    m_buffer = static_cast<DBWinBuffer*>(MapViewOfFile(m_mapping, FILE_MAP_READ, 0, 0, 4096));

    if (!m_buffer)
      return;

    m_running = true;

    m_thread = std::thread([this] {
      SetEvent(m_bufferReady);

      while (m_running) {
        if (WaitForSingleObject(m_dataReady, 100) != WAIT_OBJECT_0)
          continue;

        if (!m_running)
          break;

        if (m_buffer->processId == GetCurrentProcessId()) {
          std::fputs(m_buffer->data, stdout);
          std::fflush(stdout);
        }

        SetEvent(m_bufferReady);
      }
    });
  }

  ~DebugOutputToStdout() {
    m_running = false;

    if (m_dataReady)
      SetEvent(m_dataReady);

    if (m_thread.joinable())
      m_thread.join();

    if (m_buffer)
      UnmapViewOfFile(m_buffer);

    if (m_mapping)
      CloseHandle(m_mapping);

    if (m_dataReady)
      CloseHandle(m_dataReady);

    if (m_bufferReady)
      CloseHandle(m_bufferReady);
  }

  DebugOutputToStdout(const DebugOutputToStdout&) = delete;
  DebugOutputToStdout& operator=(const DebugOutputToStdout&) = delete;

 private:
  struct DBWinBuffer {
    DWORD processId;
    char data[4096 - sizeof(DWORD)];
  };

  HANDLE m_bufferReady = nullptr;
  HANDLE m_dataReady = nullptr;
  HANDLE m_mapping = nullptr;

  DBWinBuffer* m_buffer = nullptr;

  std::atomic<bool> m_running{false};
  std::thread m_thread;
};
}  // namespace Ngin
