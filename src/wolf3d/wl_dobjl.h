REFKEEN_NS_B

#define REFKEEN_EMULATE_DOOROBJLIST_OVERFLOWS 1

id0_byte_t DoorObjList_GetTileX(id0_int_t door);
void DoorObjList_SetTileX(id0_int_t door, id0_byte_t tilex);
id0_byte_t DoorObjList_GetTileY(id0_int_t door);
void DoorObjList_SetTileY(id0_int_t door, id0_byte_t tiley);
id0_boolean_t DoorObjList_GetVertical(id0_int_t door);
void DoorObjList_SetVertical(id0_int_t door, id0_boolean_t vertical);
id0_boolean_t DoorObjList_GetSeen(id0_int_t door);
void DoorObjList_SetSeen(id0_int_t door, id0_boolean_t seen);
id0_byte_t DoorObjList_GetLock(id0_int_t door);
void DoorObjList_SetLock(id0_int_t door, id0_byte_t lock);
// Should technically return a dooraction_t, but since there's no guarantee in
// case of overflow emulation, let's assume we can use the returned int value
id0_int_t DoorObjList_GetAction(id0_int_t door);
void DoorObjList_SetAction(id0_int_t door, dooraction_t action);
id0_int_t DoorObjList_GetTicCount(id0_int_t door);
void DoorObjList_SetTicCount(id0_int_t door, id0_int_t ticcount);

REFKEEN_NS_E
