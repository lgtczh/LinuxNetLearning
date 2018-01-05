//
// Created by 曹志昊 on 2018/1/4.
//

#ifndef LINUXNET_ERR_EXIT_H
#define LINUXNET_ERR_EXIT_H

#define ERR_EXIT(m) \
do { \
    perror(m); \
    exit(EXIT_FAILURE); \
}while(0)


#endif //LINUXNET_ERR_EXIT_H
