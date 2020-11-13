## BingoCTF 2020 Cheating Report

### Summary

*BingoCTF 2020 had a rule that asked users to refrain from playing in a crowded area which were not limited to cafes or offices.*

**It seemed like some of them broke the rule.** As an organizer, we had a long discussion about the issue and finally decided not to disclose nicknames. However, we have decided to disclose part of their IP addresses.

The reason is that:

1. BingoCTF 2020 is a small contest, and there were not many participants during the competition.
2. This project intends to warn CTF cheaters that cheating detection mechanisms can become more sophisticated these days.
3. Most of the participants had a short nickname, so it was easy to predict even if we mask part of their nicknames.
4. We really didn't want cheaters to lose reputation and feel ashamed to the public. I hope they've learned their lessons.

### Logs

**3 users have played on the same IP address. (violation of the game rule)**

|date |ip   |team_name|flag |log  |
|---	|---	|---	    |---	|---	|
| 2020-11-12 05:47:29 | ::ffff:128.134.203.* | [REDACTED-G] | intmagic from (No IP) | Success |
| 2020-11-12 03:48:51 | | [REDACTED-B] | intmagic from (No IP) | CHEAT: same_ip: [128.134.203.*] 2 teams :: [REDACTED-B],[REDACTED-A] |
| 2020-11-12 03:48:51 | 128.134.203.* | [REDACTED-A] | intmagic from (No IP) | Success |
| 2020-11-12 01:11:35 | 128.134.203.* | [REDACTED-B] | simple_memo from (No IP) | Success |
| 2020-11-12 00:30:58 | 128.134.203.* | [REDACTED-B] | intmagic from (No IP) | Success |

**2 users have played on the same IP address. (violation of the game rule)**

|date |ip   |team_name|flag |log  |
|---	|---	|---	    |---	|---	|
| 2020-11-12 04:19:19 | 220.117.200.* | [REDACTED-C] | intmagic from (No IP) | Success |
| 2020-11-12 04:19:19 | | [REDACTED-C] | intmagic from (No IP) | CHEAT: same_ip: [220.117.200.*] 2 teams :: [REDACTED-D],[REDACTED-C] |
| 2020-11-12 02:27:45 | 220.117.200.* | [REDACTED-D] | temporary from 220.117.200.* | Success |
| 2020-11-12 02:27:23 | 220.117.200.* | [REDACTED-D] | intmagic from (No IP) | Success |

**Log of testing cheating detection because (i) there was no sign of direct flag trade (ii) player had a question about the challenge.**

|date |ip   |team_name|flag |log  |
|---	|---	|---	    |---	|---	|
| 2020-11-12 19:04:27 | 14.63.106.* | [REDACTED-E] | whalerice from 14.63.106.* | CHEAT: flag_trade: [REDACTED-E] <-> [REDACTED-F] |
