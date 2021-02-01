#include <gtest/gtest.h>
#include <iostream>
#include <future>

#include "tws/Contract.h"

#include "ib_client.h"

TEST(TwsTest, Connectivity_test) {
    int	port = 4002;
	int clientId = 1;

	unsigned attempt = 0;
	printf( "Start of C++ Socket Client Test %u\n", attempt);

    atom::ib::IBClient client(2);
	for(;;)
    {
        if (!client.isConnected()) client.connect( "127.0.0.1", port, clientId);

        //! [cashcontract]
        Contract contract;
        contract.symbol = "EUR";
        contract.secType = "CASH";
        contract.currency = "GBP";
        contract.exchange = "IDEALPRO";
        

        if(client.isConnected())
        {
            std::cout<<"check STATE"<<std::endl;
            if(client.state() == atom::ib::STATE::FREE){
                std::cout<<"executing"<<std::endl;
                std::async(std::launch::async, 
                &atom::ib::IBClient::historicalDataRequests,
                &client,
                contract
                );
                std::this_thread::sleep_for(std::chrono::seconds(2));
                client.disconnect();
                break;
            }
        }
    }
    
	printf ( "End of C++ Socket Client Test\n");
}