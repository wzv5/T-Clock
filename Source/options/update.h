#ifndef TCLOCK_UPDATE_H_
#define TCLOCK_UPDATE_H_

#define UPDATE_SHOW   0
#define UPDATE_NOTIFY 1
#define UPDATE_SILENT 2

/** \brief checks for T-Clock updates
 * \param type requested type, either: 0 = show/manual, 1 = notify, 2 = silent
 * \return IDCANCEL on error, IDYES if updates were found, IDNO if there are no updates
 * \sa UPDATE_SHOW, UPDATE_NOTIFY, UPDATE_SILENT */
int UpdateCheck(int type);

#endif // TCLOCK_UPDATE_H_
