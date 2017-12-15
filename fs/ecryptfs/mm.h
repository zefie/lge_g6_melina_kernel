/*
 * mm.h
 *
 *  Created on: Feb 17, 2016
 *      Author: mudzizi
 */

#ifndef ECRYPTFS_MM_H_
#define ECRYPTFS_MM_H_

void ecryptfs_mm_drop_cache(int userid);
void ecryptfs_mm_do_sdp_cleanup(struct inode *inode);

void page_dump (struct page *p);

#endif /* ECRYPTFS_MM_H_ */
