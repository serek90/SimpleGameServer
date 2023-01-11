#ifndef BETGAMEPROTOCOL_H
#define BETGAMEPROTOCOL_H


#define BEGASEP_OPEN    1
#define BEGASEP_ACCEPT  2
#define BEGASEP_BET     3
#define BEGASEP_RESULT  4

struct BEGASEP_header
{
        uint8_t  Ver:3;
        uint8_t  Len:5;
        uint8_t  Type:8;
        uint16_t  Client_id;
};

struct BEGASEP_accept
{
        uint32_t lowNumRange;
        uint32_t upNumRange;
};

struct BEGASEP_betting
{
        struct BEGASEP_header header;
        uint32_t bettingNum;
};

struct BEGASEP_result
{
        struct BEGASEP_header header;
        uint8_t status;
        uint32_t winNum;
} __attribute__((packed));

void print_header(struct BEGASEP_header *h)
{
        printf("ver: %d, len: %d, type: %d, client_id: %d\n", h->Ver,
                                                              h->Len,
                                                              h->Type,
                                                              h->Client_id);
}

#endif /* !define  BETGAMEPROTOCOL_H */
