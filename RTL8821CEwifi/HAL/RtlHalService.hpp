/*
 * Copyright (C) 2024  RTL8821CEwifi Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef RtlHalService_hpp
#define RtlHalService_hpp

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>
#include "../RTL80211/compat.h"

/**
 * RtlHalService - Hardware Abstraction Layer for Realtek WiFi chips
 * 
 * This class provides the hardware abstraction interface between the macOS
 * IOKit layer and the Linux-derived rtw88 driver code. It handles:
 * - Memory-mapped I/O register access
 * - DMA memory allocation and management
 * - PCI configuration space access
 * - Interrupt handling
 */
class RtlHalService : public OSObject {
    OSDeclareDefaultStructors(RtlHalService)

public:
    // Hardware register access methods (8/16/32 bit)
    virtual void write8(u32 addr, u8 val) = 0;
    virtual void write16(u32 addr, u16 val) = 0;
    virtual void write32(u32 addr, u32 val) = 0;
    
    virtual u8 read8(u32 addr) = 0;
    virtual u16 read16(u32 addr) = 0;
    virtual u32 read32(u32 addr) = 0;
    
    // DMA operations
    virtual bool allocateDmaMemory(bus_size_t size, bus_dma_segment_t* seg) = 0;
    virtual void freeDmaMemory(bus_dma_segment_t* seg) = 0;
    virtual bus_addr_t getDmaAddress(bus_dma_segment_t seg) = 0;
    virtual void* getDmaVirtualAddress(bus_dma_segment_t seg) = 0;
    virtual void syncDmaMemory(bus_dma_segment_t seg, bus_size_t offset, bus_size_t len, int ops) = 0;
    
    // PCI configuration access
    virtual u32 configRead32(int offset) = 0;
    virtual void configWrite32(int offset, u32 value) = 0;
    
    // Interrupt management
    virtual bool enableInterrupt() = 0;
    virtual void disableInterrupt() = 0;
    
    // Initialization
    virtual bool init(IOPCIDevice* provider, IOWorkLoop* workLoop) = 0;
    virtual void free() override;

protected:
    IOPCIDevice*    m_pciDevice;        // PCI device provider
    IOWorkLoop*     m_workLoop;         // Work loop for interrupt handling
    bus_space_tag_t m_spaceTag;         // Memory space tag for register access
    bus_space_handle_t m_spaceHandle;   // Memory space handle
    bus_addr_t      m_baseAddr;         // Base physical address
    bus_size_t      m_size;             // Memory region size
};

#endif /* RtlHalService_hpp */
