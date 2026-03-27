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

#ifndef RTL8821CEwifi_hpp
#define RTL8821CEwifi_hpp

#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOCommandGate.h>
#include "HAL/RtlHalService.hpp"
#include "../RTL80211/compat.h"

// Forward declaration of rtw_dev from main.h
struct rtw_dev;

/**
 * RTL8821CEwifi - macOS IOKit driver for Realtek RTL8821CE PCIe WiFi adapter
 * 
 * This class extends IOEthernetController to provide macOS network driver
 * functionality while using the Linux rtw88 driver for hardware operations.
 * 
 * Architecture:
 * - Inherits from IOEthernetController (not IO80211Controller for Sequoia compatibility)
 * - Uses RtlHalService for hardware abstraction
 * - Bridges Linux rtw88 driver calls to macOS IOKit APIs
 * - Presents as an Ethernet interface to macOS (requires Heliport for WiFi UI)
 */
class RTL8821CEwifi : public IOEthernetController {
    OSDeclareDefaultStructors(RTL8821CEwifi)

public:
    // IOKit lifecycle methods
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    virtual bool attach(IOService *provider) override;
    virtual void detach(IOService *provider) override;
    virtual void free() override;

    // IOEthernetController required methods
    virtual IOReturn enable(IONetworkInterface *netif) override;
    virtual void disable(IONetworkInterface *netif) override;
    virtual IOReturn getHardwareAddress(IOEthernetAddress *addr) override;
    virtual IOReturn setHardwareAddress(const IOEthernetAddress *addr) override;
    virtual IOReturn setPromiscuousMode(bool enabled) override;
    virtual IOReturn setMulticastMode(bool enabled) override;
    virtual IOReturn setMulticastList(IOEthernetAddress *addrs, UInt32 count) override;
    
    // Packet transmission
    virtual IOReturn outputStart(IONetworkInterface *interface, IOOptionBits options) override;
    
    // Hardware statistics
    virtual void getStatistics(IOEthernetStat *stats) override;
    
    // Property management
    virtual IOReturn setProperties(OSObject *properties) override;
    
    // Power management
    virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService *whatDevice) override;
    
    // HAL service access (used by rtw_bridge.cpp)
    RtlHalService* getHalService() const { return fHalService; }

    // Interrupt handling
    static void interruptHandler(OSObject* target, IOInterruptEventSource* src, int count);
    void handleInterrupt();

    // Command gate actions (for thread-safe operations)
    static IOReturn txAction(OSObject* owner, void* arg0, void* arg1, void* arg2, void* arg3);
    static IOReturn rxAction(OSObject* owner, void* arg0, void* arg1, void* arg2, void* arg3);

protected:
    // Hardware initialization
    bool initializeHardware();
    bool setupDMA();
    bool setupInterrupts();
    void cleanupHardware();
    
    // Interface creation
    bool createNetworkInterface();
    void freeNetworkInterface();

    // Driver state
    bool fInitialized;              // Hardware initialized flag
    bool fEnabled;                  // Interface enabled flag
    bool fInterruptsInstalled;      // Interrupt handler installed
    
    // IOKit components
    IOPCIDevice*          fPCIDevice;           // PCI device provider
    IOWorkLoop*           fWorkLoop;            // Driver work loop
    IOCommandGate*        fCommandGate;         // Command gate for serialized operations
    IOInterruptEventSource* fInterruptSource;   // Interrupt event source
    IOEthernetInterface*  fNetIF;               // Network interface
    
    // Hardware abstraction layer
    RtlHalService*        fHalService;          // HAL service instance
    
    // Linux rtw88 driver state
    struct rtw_dev*     fRtwDev;                // rtw88 device structure
    
    // DMA descriptors
    bus_dma_tag_t       fDmaTag;                // DMA tag for memory allocation
    bus_dmamap_t        fTxDmaMap;              // TX DMA map
    bus_dmamap_t        fRxDmaMap;              // RX DMA map
    
    // Network statistics
    IOEthernetStat      fStats;                 // Ethernet statistics counters
    
    // MAC address
    IOEthernetAddress   fMACAddress;            // Hardware MAC address
};

#endif /* RTL8821CEwifi_hpp */
