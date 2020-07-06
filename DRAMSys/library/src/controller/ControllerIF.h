/*
 * Copyright (c) 2019, Technische Universität Kaiserslautern
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *    Kirill Bykov
 *    Lukas Steiner
 */

#ifndef CONTROLLERIF_H
#define CONTROLLERIF_H

#include <systemc.h>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include "../configuration/Configuration.h"

// Utiliy class to pass around the DRAMSys, without having to propagate the template defintions
// throughout all classes
class ControllerIF : public sc_module
{
public:
    // Already create and bind sockets to the virtual functions
    tlm_utils::simple_target_socket<ControllerIF> tSocket; // Arbiter side
    tlm_utils::simple_initiator_socket<ControllerIF> iSocket; // DRAM side

    // Destructor
    virtual ~ControllerIF()
    {
        sc_time activeTime = numberOfTransactionsServed
                             * Configuration::getInstance().memSpec->burstLength
                             / Configuration::getInstance().memSpec->dataRate
                             * Configuration::getInstance().memSpec->tCK;

        double bandwidth = (activeTime / sc_time_stamp() * 100);
        double bandwidthWoIdle = ((activeTime) / (sc_time_stamp() - idleTimeCollector.getIdleTime()) * 100);

        double maxBandwidth = (
                                  // fCK in Mhz e.g. 800 [MHz]:
                                  (1000000 / Configuration::getInstance().memSpec->tCK.to_double())
                                  // DataRate e.g. 2
                                  * Configuration::getInstance().memSpec->dataRate
                                  // BusWidth e.g. 8 or 64
                                  * Configuration::getInstance().memSpec->bitWidth
                                  //   Number of devices on a DIMM e.g. 8
                                  * Configuration::getInstance().memSpec->numberOfDevicesOnDIMM ) / ( 1024 );

        std::cout << name() << std::string("  Total Time:     ")
             << sc_time_stamp().to_string()
             << std::endl;
        std::cout << name() << std::string("  AVG BW:         ")
             << std::fixed << std::setprecision(2)
             << ((bandwidth / 100) * maxBandwidth)
             << " Gibit/s (" << bandwidth << " %)"
             << std::endl;
        std::cout << name() << std::string("  AVG BW\\IDLE:    ")
             << std::fixed << std::setprecision(2)
             << ((bandwidthWoIdle / 100) * maxBandwidth)
             << " Gibit/s (" << bandwidthWoIdle << " %)"
             << endl;
        std::cout << name() << std::string("  MAX BW:         ")
             << std::fixed << std::setprecision(2)
             << maxBandwidth << " Gibit/s"
             << std::endl;
    }

protected:
    // Bind sockets with virtual functions
    ControllerIF(sc_module_name name) :
        sc_module(name), tSocket("tSocket"), iSocket("iSocket")
    {
        tSocket.register_nb_transport_fw(this, &ControllerIF::nb_transport_fw);
        tSocket.register_transport_dbg(this, &ControllerIF::transport_dbg);
        iSocket.register_nb_transport_bw(this, &ControllerIF::nb_transport_bw);
    }
    SC_HAS_PROCESS(ControllerIF);

    // Virtual transport functions
    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &, tlm::tlm_phase &, sc_time &) = 0;
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload &) = 0;
    virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload &, tlm::tlm_phase &, sc_time &) = 0;

    // Bandwidth related
    class IdleTimeCollector
    {
    public:
        void start()
        {
            if (!isIdle)
            {
                PRINTDEBUGMESSAGE("IdleTimeCollector", "IDLE start");
                idleStart = sc_time_stamp();
                isIdle = true;
            }
        }

        void end()
        {
            if (isIdle)
            {
                PRINTDEBUGMESSAGE("IdleTimeCollector", "IDLE end");
                idleTime += sc_time_stamp() - idleStart;
                isIdle = false;
            }
        }

        sc_time getIdleTime()
        {
            return idleTime;
        }

    private:
        bool isIdle = false;
        sc_time idleTime = SC_ZERO_TIME;
        sc_time idleStart;
    } idleTimeCollector;

    uint64_t numberOfTransactionsServed = 0;
};


#endif // CONTROLLERIF_H
