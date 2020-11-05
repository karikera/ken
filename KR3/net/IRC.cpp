#include "stdafx.h"

#ifndef NO_USE_SOCKET

#include "IRC.h"
#include <KR3/data/switch.h>

using namespace kr;
using namespace irc;

User::User(Text nick) noexcept
{
	param = nullptr;
	m_IPAddress = nullptr;
	m_InCount = 0;
	m_strNick = nick;
}
User::~User() noexcept
{
}
void User::user(Text name, Text real) noexcept
{
	m_strName.copy(name);
	m_strReal.copy(real);
}
void User::nick(Text nick) noexcept
{
	m_strNick.copy(nick);
}
void User::real(Text real) noexcept
{
	m_strReal.copy(real);
}
void User::setIP(Ipv4Address ip) noexcept
{
	m_IPAddress = ip;
}
Text User::nick() noexcept
{
	return m_strNick;
}
Text User::real() noexcept
{
	return m_strReal;
}
Text User::name() noexcept
{
	return m_strName;
}

InUser::InUser(User* user, char mode) noexcept :m_user(user)
{
	m_charMode = mode;
	m_user->m_InCount++;
}
InUser::~InUser() noexcept
{
}
void InUser::mode(char mode, char sign) noexcept
{
	switch (sign)
	{
	case '+':
		if (m_charMode == mode) break;
		if (mode == '+' && m_charMode == '@') break;
		m_charMode = mode;
		break;
	case '-':
		if (m_charMode != mode) break;
		switch (mode)
		{
		case '@': m_charMode = '+'; break;
		case '+': m_charMode = ' '; break;
		case 'b': m_charMode = ' '; break;
		}
		break;
	}
}
char InUser::mode() noexcept
{
	return m_charMode;
}

Channel::Channel(Text channel) noexcept
{
	m_name.copy(channel);
}
Channel::~Channel() noexcept
{
}
InUser* Channel::join(User* user) noexcept
{
	return m_InUser.create(user, ' ');
}
void Channel::mode(User* user, char mode, char sign) noexcept
{
	findUser(user)->mode(mode, sign);
}
InUser* Channel::findUser(User* user) noexcept
{
	for(InUser &inuser : m_InUser)
	{
		if (inuser.m_user == user) return &inuser;
	}
	return nullptr;
}
size_t Channel::userCount() noexcept
{
	return m_InUser.size();
}
Text Channel::name() noexcept
{
	return m_name;
}

IRC::IRC() noexcept
{
}
IRC::~IRC() noexcept
{
	m_channels.clear();
	for (User & user : m_users)
	{
		onUserDeleted(&user);
	}
	m_users.clear();
}
IRC::IRC(pcstr16 url, word port, Text name, Text real, Text nick) noexcept
{
	ircConnect(url, port, name, real, nick);
}
bool IRC::isConnected() noexcept
{
	return getSocket() != nullptr;
}
void IRC::ircConnect(pcstr16 url, word port, Text name, Text real, Text nick) noexcept
{
	m_users.create(nick);
	ircNick(nick);
	if (name != nullptr)
	{
		m_users.front()->user(name, real);
		ircUser(name, real);
	}

	m_channels.create((Text)"");
	connect(url, port);
}

void IRC::ircSend(Text str) noexcept
{
	write(str);
	flush();
}
void IRC::ircNick(Text name) noexcept
{
	ircSend(TSZ() << "NICK " << name << "\r\n");
}

void IRC::ircReal(Text name) noexcept
{
	ircSend(TSZ() << "REAL " << name << "\r\n");
}
void IRC::ircCap(Text caps) noexcept
{
	ircSend(TSZ() << "CAP REQ :" << caps << "\r\n");
}
void IRC::ircPass(Text pass) noexcept
{
	ircSend(TSZ() << "PASS " << pass << "\r\n");
}
void IRC::ircPrivMsg(Channel* channel, Text str) noexcept
{
	ircSend(TSZ() << "PRIVMSG " << channel->m_name << " :" << str << "\r\n");
	User * user = m_users.front();
	tags.clear();
	onMessage(user, channel, str);
}
void IRC::ircJoin(Text channel) noexcept
{
	ircSend(TSZ() << "JOIN " << channel << "\r\n");
}
void IRC::ircPart(Channel* channel) noexcept
{
	ircSend(TSZ() << "PART " << channel->m_name << "\r\n");
}
void IRC::ircUser(Text name, Text real) noexcept
{
	ircSend(TSZ() << "USER " << name << " * 0 :" << real << "\r\n");
	m_users.front()->user(name, real);
}
bool IRC::ircMode(Channel* channel, User* user, char mode, char sign) noexcept
{
	InUser * inuser = channel->findUser(user);
	if (inuser == nullptr)
		return false;
	inuser->mode(mode, sign);
	ircSend(TSZ() << "MODE " << channel->m_name << " " << sign << mode << " " << user->m_strNick << "\r\n");
	return true;
}
void IRC::ircKick(Channel* channel, User* user) noexcept
{
	ircSend(TSZ() << "KICK " << channel->m_name << user->m_strNick << "\r\n");
}
void IRC::ircQuit(Text str) noexcept
{
	if (str == nullptr)
		ircSend((Text)"QUIT\r\n");
	else 
		ircSend(TSZ() << "QUIT :" << str << "\r\n");
	close();
}
User* IRC::ircFindUser(Text nick) noexcept
{
	for(User &user : m_users)
	{
		if (user.m_strNick == nick) return &user;
	}

	User * user = m_users.create(nick);
	onUserCreated(user);
	return user;
}
Channel* IRC::ircFindChannel(Text chname) noexcept
{
	for(Channel &channel : m_channels)
	{
		if (channel.m_name.equals(chname)) return &channel;
	}
	return m_channels.create(chname);
}

void IRC::onUserCreated(User * user) noexcept
{
}
void IRC::onUserDeleted(User * user) noexcept
{
}
void IRC::onIRCConnect() noexcept
{
}
void IRC::onLine(Text line) noexcept
{
}
void IRC::onNotice(int num, Text message) noexcept
{
}
void IRC::onMessage(User * user, Channel * channel, Text message) noexcept
{
}
void IRC::onJoin(InUser * user) noexcept
{
}
void IRC::onOut(Channel * channel, User * user) noexcept
{
}
void IRC::onNick(User * user, Text nick) noexcept
{
}
void IRC::onJoinOper(InUser * user) noexcept
{
}
void IRC::onUnprocessed(User * user, View<Text> messages) noexcept
{
}

Text IRC::getNick() noexcept
{
	return m_users.front()->nick();
}
ChannelList& IRC::getChannelList() noexcept
{
	return m_channels;
}
UserList& IRC::getUserList() noexcept
{
	return m_users;
}
void IRC::onError(Text name, int code) noexcept
{
}
void IRC::onConnect() noexcept
{
}
void IRC::onRead() noexcept
{
	try
	{
		for (;;)
		{
			TText temp;
			Text line = m_receive.retype<char>()->readwith('\n', &temp);
			if (line == nullptr) return;
			if (line.endsWith('\r')) line.addEnd(-1);

			Text sender = nullptr;

			onLine(line);

			if (*line == '@')
			{
				line++;
				Text tagparse = line.readwith(' ');
				while (!tagparse.empty())
				{
					Text readed = tagparse.readwith_e(';');
					if (readed == nullptr) break;
					Text varname = readed.readwith('=');
					if (varname == nullptr)
					{
						dout << "invalid tag: " << readed << endl;
						continue;
					}
					tags.insert(varname, readed);
				}
			}
			User * user = nullptr;
			if (*line == ':')
			{
				line++;
				sender = line.readwith(' ');
				Text nickname = sender.readwith('!');
				if (nickname != nullptr)
				{
					Text realname = sender.readwith('@');
					if (realname != nullptr)
					{
						user = ircFindUser(nickname);
						user->setIP(Ipv4Address(sender));
					}
				}
			}

			ParameterA param(line);
			if (sender == nullptr)
			{
				static const TextSwitch SystemMsg = { "PING", "NOTICE" };
				switch (SystemMsg[param[0]])
				{
				case 0:
					ircSend(TSZ() << "PONG :" << param[1] << "\r\n");
					break;
				case 1:
					onNotice(0, param[2]);
					break;
				}
			}
			else if (param[0].equals("NOTICE"))
			{
				onNotice(0, param[2]);
			}
			else if (param[0].numberonly())
			{
				int n;
				switch (n = param[0].to_uint())
				{
				case IRCM_NICKLIST:
				{
					Channel * channel = ircFindChannel(param[3]);
					ParameterA param2(param[4]);

					for (Text nick : param2)
					{
						char Mode = nick[0];
						if (Text("~!@#$%^&*()+`-=\\|[]{};\':\",./<>?").find(Mode) != nullptr)
							nick++;
						else
							Mode = ' ';
						User * u = ircFindUser(nick);
						if (u == m_users.front()) continue;
						onJoin(channel->m_InUser.create(u, Mode));
					}
					break;
				}
				case IRCM_REGIST:
					onIRCConnect();
					break;
				}
				onNotice(n, param.back());
			}
			else
			{
				static const TextSwitch UserMsg = { "PRIVMSG", "JOIN", "QUIT", "NICK", "PART", "JOINOPER" };

				Channel * channel;
				int order = (int)UserMsg[param[0]];
				switch (order)
				{
				case 0:
					channel = ircFindChannel(param[1]);
					onMessage(user, channel, param[2]);
					break;
				case 1:
					channel = ircFindChannel(param[1]);
					onJoin(channel->join(user));
					break;
				case 2:
					onOut(nullptr, user);
					_quit(user);
					break;
				case 3:
					onNick(user, param[1]);
					user->nick(param[1]);
					break;
				case 4:
				{
					channel = ircFindChannel(param[1]);
					onOut(channel, user);
					if (user == m_users.front())
						delete channel;
					else
					{
						_deleteInUser(channel, channel->findUser(user));
					}
					break;
				}
				case 5:
				{
					channel = ircFindChannel(param[1]);
					InUser * inuser = channel->findUser(user);
					onJoinOper(inuser);
					inuser->mode('@', '+');
					break;
				}
				default:
					onUnprocessed(user, param);
					break;
				}
			}
			tags.clear();
		}
	}
	catch (NotEnoughSpaceException&)
	{
		debug(); // 파라미터 수가 32개를 넘었다.
	}
}
void IRC::onClose() noexcept
{
}

void IRC::_deleteInUser(Channel* channel, InUser * inuser) noexcept
{
	User * user = inuser->m_user;
	channel->m_InUser.remove(inuser);

	user->m_InCount--;
	if (user->m_InCount == 0)
	{
		onUserDeleted(user);
		m_users.remove(user);
	}
}
void IRC::_quit(User * user) noexcept
{
	for (Channel &channel : m_channels)
	{
		InUser *InUser = nullptr;
		while (InUser = channel.findUser(user))
		{
			_deleteInUser(&channel, InUser);
		}
	}
}

#endif