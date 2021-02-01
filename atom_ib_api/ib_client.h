#ifndef ATOM_IB_ATOM_IB_API_IB_CLIENT_H
#define ATOM_IB_ATOM_IB_API_IB_CLIENT_H

// #include <optional>
#include <utility>
#include <variant>
#include <type_traits>
#include <future>
#include <iostream>

#include "tws/EWrapper.h"
#include "tws/EReaderOSSignal.h"
#include "tws/EReader.h"
#include "tws/EClientSocket.h"

namespace atom::ib{

	enum class STATE{
		BUSY,
		FREE
	};

	class IBClient: public EWrapper
	{
	public:
		IBClient(int sig_timeout);
		IBClient();
		~IBClient() = default;

		bool connect(const char * host, int port, int clientId = 0);
		void disconnect();
		bool isConnected();

		void processMsg(){
				signal_.waitForSignal();
				errno = 0;
				reader_->processMsgs();
		}
		void historicalDataRequests(const Contract& c)
		{
			std::thread t(
				&IBClient::_historicalDataRequests, 
				this,
				std::ref(c)
			);
    		t.join();

		}

		void _historicalDataRequests(const Contract& c)
		{
			int atempts = 0;
			while(true)
			{
				if (state_ == STATE::BUSY) {
					std::cout<< "state is busy! Wait 3 Seconds"<<std::endl;

					std::this_thread::sleep_for(std::chrono::seconds(3));
					if (atempts >3) break;
					++atempts;
					continue;
				}
				std::cout<< "Requesting Historical Data"<<std::endl;
				client_.reqHistoricalData(
					4001, 
					c, 
					"20201230 17:00:00", 
					"1 M", 
					"1 day", 
					"MIDPOINT", 
					1, 
					1, 
					false, 
					TagValueListSPtr()
				)	;
				std::this_thread::sleep_for(std::chrono::seconds(5));
				state_ = STATE::BUSY;

				processMsg();
				break;
			}
		}

		const auto& bar()
		{
			return bar_;
		}

		const auto& state()
		{
			return state_;
		}

		//override functions from EWrapper
		#include "tws/EWrapper_prototypes.h"
	private:

		EReaderOSSignal signal_;
		EClientSocket client_;
		std::unique_ptr<EReader> reader_;
		std::vector<Bar> bar_;
		STATE state_;
	};
}

#endif //ATOM_IB_ATOM_IB_API_IB_CLIENT_H