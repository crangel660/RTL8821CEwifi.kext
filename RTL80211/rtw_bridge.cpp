#include "RTL8821CEwifi.hpp"
#include "HAL/RtlHalService.hpp"

// Solo incluimos los HEADERS (.h), nunca los .c
extern "C" {
    #include "../RTL8821CE/main.h"  // Donde está definido rtw_dev
    #include "../RTL8821CE/pci.h"   // Donde está definido rtw_pci

    // Aquí implementamos las funciones que los .c de Realtek
    // van a buscar cuando se estén enlazando.

    void rtw_write32(struct rtw_dev *rtwdev, u32 addr, u32 val) {
        RTL8821CEwifi *me = (RTL8821CEwifi *)rtwdev->priv;
        if (me && me->fHalService) {
            me->fHalService->write32(addr, val);
        }
    }

    u32 rtw_read32(struct rtw_dev *rtwdev, u32 addr) {
        RTL8821CEwifi *me = (RTL8821CEwifi *)rtwdev->priv;
        return (me && me->fHalService) ? me->fHalService->read32(addr) : 0;
    }

    // Soporte para 8 y 16 bits (Crucial para calibración de antena)
    void rtw_write8(struct rtw_dev *rtwdev, u32 addr, u8 val) {
        RTL8821CEwifi *me = (RTL8821CEwifi *)rtwdev->priv;
        if (me && me->fHalService) me->fHalService->write8(addr, val);
    }

    void rtw_write16(struct rtw_dev *rtwdev, u32 addr, u16 val) {
        RTL8821CEwifi *me = (RTL8821CEwifi *)rtwdev->priv;
        if (me && me->fHalService) me->fHalService->write16(addr, val);
    }

   // Función que rx.c llamará cuando tenga un paquete listo descifrado
  void rtw_os_indicate_packet(struct rtw_dev *rtwdev, struct sk_buff *skb) {
    RTL8821CEwifi *me = (RTL8821CEwifi *)rtwdev->priv;

    if (me && skb && skb->m) {
        // Sincronizar longitud final
        mbuf_pkthdr_setlen(skb->m, skb->len);
        mbuf_setlen(skb->m, skb->len);

        // Entregar a la pila de red de Apple
        // Al heredar de IOEthernetController, necesitamos pasar por la interfaz de red
        if (me->fNetIF) {
            me->fNetIF->inputPacket(skb->m, skb->len, 0, NULL);
        } else {
            // Fallback: liberar el paquete si no hay interfaz
            mbuf_freem(skb->m);
        }

        // Desvincular mbuf del skb para que kfree(skb) no borre el mbuf
        skb->m = NULL;
    }

    // Liberar solo la estructura envoltorio
    kfree(skb);
}
}