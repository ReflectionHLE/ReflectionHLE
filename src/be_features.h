#ifndef BE_FEATURES_H
#define BE_FEATURES_H

/* This header is used to define features, based on the EXE being built */

#if (defined REFKEEN_VER_CATARM) || (defined REFKEEN_VER_CATAPOC)
#define BE_CROSS_ENABLE_FARPTR_CFG 1
#endif

#endif // BE_FEATURES_H
