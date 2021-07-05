/* Wolfenstein 3D Source Code
 * Copyright (C) 1993-1996 id Software, Inc.
 * Copyright (C) 2020-2021 NY00123
 *
 * This file is part of Wolfenstein 3D.
 *
 * Wolfenstein 3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Wolfenstein 3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an addendum, you can further use the Wolfenstein 3D Source Code under
 * the terms of the limited use software license agreement for Wolfenstein 3D.
 * See id-wolf3d.txt for these terms.
 */

#include "wl_def.h"

// Ported from ASM

#ifdef GAMEVER_NOAH3D // REFKEEN: Limit compiled code to S3DNA only for now
REFKEEN_NS_B

//static const id0_char_t
const id0_char_t *Question[] = {
//* const Quiz001 =
		"Who was Noah's father?\x00"
		"Gen 5:30\x00"
		"Methuselah\x02"
		"Lamech\x01"
		"Enoch\x04",
//* const Quiz002 =
		"How old was Noah when he had\n"
		"Shem, Ham and Japheth?\x00"
		"Gen 5:32\x00"
		"100\x02"
		"500\x01"
		"700\x04",
//* const Quiz003 =
		"How many sons did Noah have?\x00"
		"Gen 6:10\x00"
		"3\x01"
		"5\x02"
		"7\x04",
//* const Quiz004 =
		"What kind of wood was the Ark\n"
		"made of?\x00"
		"Gen 6:14\x00"
		"oak\x02"
		"pine\x02"
		"gopher wood\x03",
//* const Quiz005 =
		"How many cubits long was the\n"
		"Ark?\x00"
		"Gen 6:15\x00"
		"200\x02"
		"300\x01"
		"500\x04",
//* const Quiz006 =
		"How many windows did the Ark\n"
		"have?\x00"
		"Gen 6:16\x00"
		"1\x01"
		"10\x02"
		"20\x04",
//* const Quiz007 =
		"How many doors did the Ark\n"
		"have?\x00"
		"Gen 6:16\x00"
		"1\x01"
		"3\x02"
		"6\x04",
//* const Quiz008 =
		"How many stories did the Ark\n"
		"have?\x00"
		"Gen 6:16\x00"
		"2\x02"
		"3\x01"
		"7\x04",
//* const Quiz009 =
		"How many of each kind of\n"
		"animal did Noah have on\n"
		"the Ark?\x00"
		"Gen 6:19\x00"
		"2\x01"
		"3\x02"
		"5\x04",
//* const Quiz010 =
		"How many of each clean animal\n"
		"did Noah have on the Ark?\x00"
		"Gen 7:2\x00"
		"2\x02"
		"4\x02"
		"7\x03",
//* const Quiz011 =
		"How many days did it rain?\x00"
		"Gen 7:4\x00"
		"30\x02"
		"40\x01"
		"100\x04",
//* const Quiz012 =
		"How old was Noah when the\n"
		"floods came?\x00"
		"Gen 7:6\x00"
		"500\x02"
		"600\x01"
		"650\x04",
//* const Quiz013 =
		"How many days did Noah wait\n"
		"in the Ark for the floods to\n"
		"come?\x00"
		"Gen 7:10\x00"
		"1\x02"
		"4\x02"
		"7\x03",
//* const Quiz014 =
		"The flood waters only came\n"
		"from the sky.\x00"
		"Gen 7:11\x00"
		"True\x02"
		"False\x03",
//* const Quiz015 =
		"How many people were on the\n"
		"Ark?\x00"
		"Gen 7:13\x00"
		"4\x02"
		"6\x02"
		"8\x03",
//* const Quiz016 =
		"Noah collected the animals and\n"
		"put them in the Ark.\x00"
		"Gen 7:15\x00"
		"True\x02"
		"False\x03",
//* const Quiz017 =
		"Who survived the flood?\x00"
		"Gen 7:23\x00"
		"animals on the earth\x02"
		"animals in the sky\x02"
		"animals in the Ark\x03",
//* const Quiz018 =
		"How long did the flood last?\x00"
		"Gen 7:24, Gen 8:3\x00"
		"40 days\x02"
		"150 days\x01"
		"365 days\x04",
//* const Quiz019 =
		"Which mountain did the Ark\n"
		"land on?\x00"
		"Gen 8:4\x00"
		"Ararat\x01"
		"St. Helen\x02"
		"Fuji\x04",
//* const Quiz020 =
		"What was the first animal to\n"
		"leave the Ark?\x00"
		"Gen 8:7\x00"
		"dove\x02"
		"raven\x01"
		"eagle\x04",
//* const Quiz021 =
		"What did the dove bring back\n"
		"to Noah?\x00"
		"Gen 8:11\x00"
		"an apple\x02"
		"an olive branch\x01"
		"a fig leaf\x04",
//* const Quiz022 =
		"How many days did Noah stay\n"
		"in the Ark after the dove\n"
		"brought the branch?\x00"
		"Gen 8:12\x00"
		"3\x02"
		"5\x02"
		"7\x03",
//* const Quiz023 =
		"What was the first thing that\n"
		"Noah did after he got out of\n"
		"the Ark?\x00"
		"Gen 8:20\x00"
		"built a house\x02"
		"built an altar\x01"
		"went swimming\x04",
//* const Quiz024 =
		"What is the sign that God\n"
		"gave us to show He would\n"
		"never flood the world again?\x00"
		"Gen 9:14-15\x00"
		"the rainbow\x01"
		"the blue sky\x02"
		"fire\x04",
//* const Quiz025 =
		"How old was Noah when he\n"
		"died?\x00"
		"Gen 9:29\x00"
		"750\x02"
		"890\x02"
		"950\x03",
//* const Quiz026 =
		"Who closed the door of the\n"
		"Ark when the floods came?\x00"
		"Gen 7:16\x00"
		"Noah\x02"
		"Shem\x02"
		"God\x03",
//* const Quiz027 =
		"Why did God have the floods\n"
		"come?\x00"
		"Gen 6:12-13\x00"
		"Man was wicked.\x01"
		"The earth was dirty.\x02"
		"The plants needed water.\x04",
//* const Quiz028 =
		"Noah was the only man to find\n"
		"grace in the eyes of the Lord.\x00"
		"Gen 6:8, Gen 6:11-12\x00"
		"True\x01"
		"False\x04",
//* const Quiz029 =
		"How many years did God say He\n"
		"would strive with man?\x00"
		"Gen 6:3\x00"
		"10\x02"
		"50\x02"
		"120\x03",
//* const Quiz030 =
		"What did Noah do after the\n"
		"flood?\x00"
		"Gen 9:20\x00"
		"He planted a vineyard.\x01"
		"He herded sheep.\x02"
		"He ran a caravan. \x04",
//* const Quiz031 =
		"Who was Noah's oldest son?\x00"
		"Gen 10:21\x00"
		"Ham\x02"
		"Shem\x02"
		"Japheth\x03",
//* const Quiz032 =
		"On which side did God tell\n"
		"Noah to pitch the Ark?\x00"
		"Gen 6:14\x00"
		"inside\x02"
		"outside\x02"
		"both\x03",
//* const Quiz033 =
		"How wide was the Ark?\x00"
		"Gen 6:15\x00"
		"30 Cubits\x02"
		"50 Cubits\x01"
		"60 Cubits\x04",
//* const Quiz034 =
		"How tall was the Ark?\x00"
		"Gen 6:15\x00"
		"30 Cubits\x01"
		"40 Cubits\x02"
		"50 Cubits\x04",
//* const Quiz035 =
		"What did God make with Noah,\n"
		"so that Noah knew that he\n"
		"would be safe?\x00"
		"Gen 6:18\x00"
		"a lifesaver\x02"
		"a boat\x02"
		"a covenant\x03",
//* const Quiz036 =
		"Besides animals, what other\n"
		"things did God tell Noah to\n"
		"have in the Ark?\x00"
		"Gen 6:18-21\x00"
		"food & his family\x01"
		"books & his family\x02"
		"swimsuit & a rubber duck\x04",

// *** S3DNA RESTORATION - A minor mistake, "037" wasn't used

//* const Quiz038 =
		"Why did God choose Noah to be\n"
		"in the Ark?\x00"
		"Gen 7:1\x00"
		"Noah was righteous.\x01"
		"Noah was a good sailor.\x02"
		"Noah could swim.\x04",
//* const Quiz039 =
		"Noah entered the Ark early to\n"
		"prepare for his family.\x00"
		"Gen 7:13\x00"
		"True\x02"
		"False\x03",
//* const Quiz040 =
		"How did God make the floods\n"
		"go away?\x00"
		"Gen 8:1\x00"
		"with the wind\x01"
		"with big drains\x02"
		"with large buckets\x04",
//* const Quiz041 =
		"How many months did Noah wait\n"
		"in the Ark after it came to\n"
		"rest on Mt. Ararat?\x00"
		"Gen 8:4-14\x00"
		"1.5 months\x02"
		"4.5 months\x02"
		"7.5 months\x03",
//* const Quiz042 =
		"Who was the first man off of\n"
		"the Ark?\x00"
		"Gen 8:18\x00"
		"Noah\x01"
		"Shem\x02"
		"Ham\x04",
//* const Quiz043 =
		"What did Noah offer to God as\n"
		"thanks for keeping him safe?\x00"
		"Gen 8:20\x00"
		"himself\x02"
		"animals\x01"
		"food\x04",
//* const Quiz044 =
		"What did God promise never to\n"
		"do again?\x00"
		"Gen 9:11\x00"
		"flood the earth\x01"
		"punish man\x02"
		"turn on his shower\x04",
//* const Quiz045 =
		"In what book of the Bible\n"
		"do you find the story of\n"
		"Noah?\x00"
		"No hint\x00"
		"Genesis\x01"
		"Exodus\x02"
		"Deuteronomy\x04",
//* const Quiz046 =
		"Which of Noah's ancestors lived\n"
		"to be 969 years old?\x00"
		"Gen 5:27\x00"
		"Enoch\x02"
		"Adam\x02"
		"Methuselah\x03",
//* const Quiz047 =
		"Noah is mentioned at least 3\n"
		"times in the New Testament.\x00"
		"Heb 11:7, 1 Pet 3:20,\n"
		"2 Pet 2:5\x00"
		"True\x01"
		"False\x04",
//* const Quiz048 =
		"Which of Noah's descendants\n"
		"was called 'mighty upon the\n"
		"earth'?\x00"
		"1 Chr 1:10\x00"
		"Cush\x02"
		"Nimrod\x01"
		"Elam\x04",
//* const Quiz049 =
		"Which of Noah's\n"
		"ancestors never died?\x00"
		"Gen 5:24\x00"
		"Adam\x02"
		"Enoch\x01"
		"Jared\x04",
//* const Quiz050 =
		"Noah is a descendent of\n"
		"which of Adam's sons?\x00"
		"Gen 5\x00"
		"Cain\x02"
		"Abel\x02"
		"Seth\x03",
//* const Quiz051 =
		"Why is Noah mentioned in\n"
		"Hebrews 11?\x00"
		"Heb 11:7\x00"
		"for his faith\x01"
		"for his carpentry skills\x02"
		"for his sacrifices\x04",
//* const Quiz052 =
		"Noah took his wife, sons,\n"
		"and daughters into the\n"
		"Ark.\x00"
		"Gen 7:7\x00"
		"True\x02"
		"False\x03",
//* const Quiz053 =
		"The flood waters came from\n"
		"the fountains of the deep\n"
		"and windows of heaven.\x00"
		"Gen 7:11, Gen 8:2\x00"
		"True\x01"
		"False\x04",
//* const Quiz054 =
		"What is a covenant?\x00"
		"No hint\x00"
		"a contract\x01"
		"a book\x02"
		"an altar\x04",
//* const Quiz055 =
		"What title is Noah given\n"
		"in 2 Peter 2:5?\x00"
		"No hint\x00"
		"man after God's own heart\x02"
		"preacher of righteousness\x01"
		"an apostle\x04",
//* const Quiz056 =
		"The Gospels liken the time\n"
		"before the flood to the time:\n"
		"before:\x00"
		"Matt 24:37-38, Luke 17:26-27\x00"
		"the Incarnation\x02"
		"the Second Coming\x01"
		"the Cruxifiction\x04",
//* const Quiz057 =
		"By the time of Noah, how did\n"
		"God feel about having created\n"
		"man?\x00"
		"Gen 6:6\x00"
		"It pleased Him.\x02"
		"It grieved Him.\x01"
		"It amused Him.\x04",
//* const Quiz058 =
		"Jesus is a descendant of\n"
		"Noah.\x00"
		"Luke 3:36\x00"
		"True\x01"
		"False\x04",
//* const Quiz059 =
		"What kind of creatures were\n"
		"NOT mentioned as destroyed\n"
		"in the flood?\x00"
		"Gen 7:21-22\x00"
		"birds\x02"
		"sea creatures\x01"
		"livestock\x04",
//* const Quiz060 =
		"By how much did the water\n"
		"cover the mountain-tops?\x00"
		"Gen 7:20\x00"
		"7 cubits\x02"
		"15 cubits\x01"
		"40 cubits\x04",
//* const Quiz061 =
		"What day of the seventh\n"
		"month did the Ark come to\n"
		"rest on Ararat?\x00"
		"Gen 8:4\x00"
		"the 7th day\x02"
		"the 17th day\x01"
		"the 24th day\x04",
//* const Quiz062 =
		"What was Noah commanded\n"
		"not to eat?\x00"
		"Gen 9:4\x00"
		"ham\x02"
		"brussel sprouts\x02"
		"blood\x03",
//* const Quiz063 =
		"After the flood, what were\n"
		"the animals supposed to do?\x00"
		"Gen 8:17\x00"
		"exercise and become strong\x02"
		"be fruitful and multiply\x01"
		"be sacrificed to God\x04",
//* const Quiz064 =
		"With whom did God make His\n"
		"covenant?\x00"
		"Gen 9:8-10\x00"
		"Noah\x02"
		"Noah and his sons\x02"
		"Noah, his sons and every\n"
		"   living creature\x03",
//* const Quiz065 =
		"Who was the father of the\n"
		"Canaanites?\x00"
		"Gen 9:22\x00"
		"Ham\x01"
		"Shem\x02"
		"Japheth\x04",
//* const Quiz066 =
		"Abraham is a descendent of\n"
		"which son of Noah?\x00"
		"Gen 11:10-27\x00"
		"Ham\x02"
		"Shem\x01"
		"Japheth\x04",
//* const Quiz067 =
		"Which of Noah's descendants\n"
		"did he curse?\x00"
		"Gen 9:25\x00"
		"Canaan\x01"
		"Shem\x02"
		"Japheth\x04",
//* const Quiz068 =
		"Why did Shem and Japheth\n"
		"cover up Noah with a\n"
		"garment?\x00"
		"Gen 9:23\x00"
		"Noah was naked\x01"
		"Noah was cold\x02"
		"Noah was burning from\n"
		"    the sun\x04",
//* const Quiz069 =
		"The rainbow is a symbol that\n"
		"God will never judge mankind\n"
		"again.\x00"
		"Gen 9:15\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz070 =
		"God promised that Noah and\n"
		"the animals would get along.\x00"
		"Gen 9:2\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz071 =
		"Man did not eat meat before\n"
		"the flood.\x00"
		"Gen 9:3\x00"
		"True\x01"
		"False\x04",
//* const/ Quiz072 =
		"What did God say would\n"
		"happen to the man who\n"
		"commits murder?\x00"
		"Gen 9:6\x00"
		"He will be killed.\x01"
		"He will be cursed.\x02"
		"He will be put in jail for\n"
		"   the rest of his life.\x04",
//* const/ Quiz073 =
		"Everyone on the Ark survived\n"
		"the flood.\x00"
		"Gen 8:18\x00"
		"True\x01"
		"False\x04",
//* const/ Quiz074 =
		"How long did it take the\n"
		"water to recede so that the\n"
		"Ark could rest on Mount\n"
		"Ararat?\x00"
		"Gen 8:3\x00"
		"100 days\x02"
		"150 days\x01"
		"300 days\x04",
//* const/ Quiz075 =
		"After the flood, God\n"
		"destroyed the Ark.\x00"
		"No hint\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz076 =
		"Noah had to gather and store\n"
		"food to eat during the flood\n"
		"for:\x00"
		"Gen 7:20\x00"
		"himself and his family\x02"
		"the animals\x02"
		"both\x03",
//* const/ Quiz077 =
		"The Bible says that Noah's\n"
		"wife was named Sarah.\x00"
		"No hint\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz078 =
		"After the flood, Noah did not\n"
		"have any more sons.\x00"
		"Gen 9:19\x00"
		"True\x01"
		"False\x04",
//* const/ Quiz079 =
		"How many years did God give\n"
		"mankind to repent before the\n"
		"flood?\x00"
		"Gen 6:3\x00"
		"70\x02"
		"100\x02"
		"120\x03",
//* const/ Quiz080 =
		"Which person lived before\n"
		"Noah?\x00"
		"Gen 5\x00"
		"Abraham\x02"
		"Adam\x01"
		"Solomon\x04",
//* const/ Quiz081 =
		"Which person lived after Noah?\x00"
		"No hint\x00"
		"Cain\x02"
		"Methuselah\x02"
		"Joshua\x03",
//* const/ Quiz082 =
		"Noah did everything that God\n"
		"told him to do.\x00"
		"Gen 6:22\x00"
		"True\x01"
		"False\x04",
//* const/ Quiz083 =
		"After Noah, with whom did God\n"
		"make His next covenant?\x00"
		"Gen 15\x00"
		"Abraham\x01"
		"David\x02"
		"Moses\x04",
//* const/ Quiz084 =
		"What kind of covenant did God\n"
		"make with Noah?\x00"
		"Gen 9:8-17\x00"
		"conditional\x02"
		"unconditional\x01"
		"temporary\x04",
//* const/ Quiz085 =
		"People did not eat vegetables\n"
		"before the flood.\x00"
		"Gen 9:3\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz086 =
		"In the covenant, God said that\n"
		"seedtime and harvest, cold and\n"
		"heat, summer and winter, and\n"
		"what else would not cease?\x00"
		"Gen 8:22\x00"
		"day and night\x01"
		"living and dying\x02"
		"land and seas\x04",
//* const/ Quiz087 =
		"God told Noah to make windows\n"
		"for all of the animals.\x00"
		"Gen 6:16\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz088 =
		"Rainbows existed before the\n"
		"flood.\x00"
		"Gen 9:12-17\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz089 =
		"About how long is a cubit?\x00"
		"No hint\x00"
		"10 inches\x02"
		"18 inches\x01"
		"3 feet\x04",
//* const/ Quiz090 =
		"It took Noah 40 days to find\n"
		"two of every animal to put\n"
		"inside the Ark.\x00"
		"Gen 7:8-9\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz091 =
		"On what day of the month did\n"
		"Noah look out of the Ark and\n"
		"see dry land?\x00"
		"Gen 8:13\x00"
		"the first day\x01"
		"the third day\x02"
		"the last day\x04",
//* const/ Quiz092 =
		"In Noah's curse he said\n"
		"Canaan would be a slave\n"
		"to his brethren?\x00"
		"Gen 9:25\x00"
		"True\x01"
		"False\x04",
//* const/ Quiz093 =
		"How many years did Noah\n"
		"live after the flood?\x00"
		"Gen 9:28\x00"
		"200\x02"
		"350\x01"
		"400\x04",
//* const/ Quiz094 =
		"Which of Noah's sons was\n"
		"a tattle-tale?\x00"
		"Gen 9:22\x00"
		"Ham\x01"
		"Shem \x02"
		"Japheth\x04",
//* const/ Quiz095 =
		"What did Noah plant after\n"
		"the flood?\x00"
		"Gen 9:20\x00"
		"an orchard\x02"
		"a vineyard\x01"
		"tomatoes\x04",
//* const/ Quiz096 =
		"How did God communicate\n"
		"to Noah?\x00"
		"Gen 6:13\x00"
		"burning bush\x02"
		"spoke directly\x01"
		"sent an angel\x04",
//* const/ Quiz097 =
		"The Ark was built with\n"
		"sails so God could move it.\x00"
		"No hint\x00"
		"True\x02"
		"False\x03",
//* const/ Quiz098 =
		"A male and female of each\n"
		"animal was brought on to\n"
		"the Ark.\x00"
		"Gen 6:19\x00"
		"True\x01"
		"False\x04",
//* const/ Quiz099 =
		"How many times did Noah\n"
		"send the dove out of the\n"
		"Ark?\x00"
		"Gen 8:8-12\x00"
		"2\x02"
		"3\x01"
		"4\x04",
//* const/ Quiz100 =
		"How old was Lamech when\n"
		"he had Noah?\x00"
		"Gen 5:28\x00"
		"135\x02"
		"182\x01"
		"199\x04",
//* const/ Quiz999 =
		""
};

REFKEEN_NS_E
#endif
