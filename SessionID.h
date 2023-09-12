#pragma once

namespace server_baby
{
    class Session;
    //=======================
    //���� ID ����ü
    //=======================
    union SessionID
    {
        struct Element_
        {
            unsigned long unique_;
            unsigned int index_;
        } element_;
        unsigned long long total_;

        SessionID() : total_(0) {}
        SessionID(Session* session)
        {
            total_ = (unsigned long long)session;
        }

        SessionID& operator=(const SessionID& rhs) {

            this->total_ = rhs.total_;
            return *this;
        }
    };
}
