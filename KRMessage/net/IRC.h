#pragma once

#include <KR3/main.h>
#include <KR3/data/linkedlist.h>
#include <KR3/data/map.h>

#include "wsevent.h"
#include "client.h"

namespace kr
{
	namespace irc
	{
		namespace __pri__
		{
			template <typename T> 
			struct __TYPE_CONST
			{
			};

			template <> struct __TYPE_CONST<byte>
			{
				byte limitof = 253;
				byte maxof = 126;
			};
			template <> struct __TYPE_CONST<word>
			{
				word limitof = 253U * 253;
				word maxof = 32004;
			};
			template <> struct __TYPE_CONST<dword>
			{
				dword limitof = 253U * 253 * 253 * 253;
				dword maxof = 2048576040;
			};
		}

#define irc_limitof(type)	(::kr::irc::__pri__::__TYPE_CONST<type>::limitof)
#define irc_maxof(type)		(::kr::irc::__pri__::__TYPE_CONST<type>::maxof)
#define irc_minof(type)		(::kr::irc::__pri__::__TYPE_CONST<type>::maxof+1)

		constexpr uint IRC_IDMAX = 256;
		constexpr uint IRCM_REGIST = 1;
		constexpr uint IRCM_SAMENICK = 433;
		constexpr uint IRCM_NICKLIST = 353;
		constexpr uint IRCM_NICKLIST_END = 366;
		constexpr uint IRCM_ERRNICK = 432;
		constexpr uint IRCM_ERRNAME = 468;

		class User;
		class InUser;
		class Channel;
		class IRC;
		template <typename C> class ParameterT;

		using ChannelList = Chain<Channel>;
		using UserList = Chain<User>;
		using InUserList = LinkedList<InUser>;
		using ParameterA = ParameterT<char>;
		using ParameterW = ParameterT<char16>;
		using Parameter16 = ParameterT<char16>;
		using Parameter32 = ParameterT<char32>;

		template <typename C>
		class ParameterT: public BArray<Text, 32>
		{
			using Text = View<C>;
		public:

			//NotEnoughSpaceException
			ParameterT(Text str) noexcept;

			using BArray<Text, 32>::operator [];
		};

		class User :public Node<User, true>
		{
			friend class InUser;
			friend class IRC;
		public:

			User(Text nick) noexcept;
			~User() noexcept;
			User(const User&) = delete;
			User& operator =(const User&) = delete;

			void user(Text name, Text real) noexcept;
			void nick(Text nick) noexcept;
			void real(Text real) noexcept;
			void setIP(Ipv4Address ip) noexcept;

			Text nick() noexcept;
			Text real() noexcept;
			Text name() noexcept;

			void * param;

		protected:
			BText<IRC_IDMAX> m_strNick, m_strReal, m_strName;
			Ipv4Address m_IPAddress;
			dword m_InCount;

		};

		class InUser:public Node<InUser, true>
		{
			friend class Channel;
			friend class IRC;
		public:

			InUser(User* user, char mode) noexcept;
			~InUser() noexcept;

			void mode(char mode, char sign) noexcept;
			void kick(User* user) noexcept;
			char mode() noexcept;

		protected:
			User* m_user;
			char m_charMode;


		};

		class Channel:public Node<Channel, true>
		{
			friend class IRC;
		public:
			Channel(Text channel) noexcept;
			~Channel() noexcept;
			InUser* join(User* user) noexcept;
			void mode(User* user, char mode, char sign) noexcept;
			InUser* findUser(User* user) noexcept;
			size_t userCount() noexcept;

			Text name() noexcept;

		protected:
			BText<IRC_IDMAX> m_name;
			InUserList m_InUser;
		};

		class IRC:private Client
		{
		public:
			using Client::makeProcedure;

			IRC() noexcept;
			~IRC() noexcept;
			IRC(pcstr16 url, word port, Text name, Text real, Text nick) noexcept;

			bool isConnected() noexcept;
			void ircConnect(pcstr16 url, word port, Text name, Text real, Text nick) noexcept;
			void ircSend(Text str) noexcept;
			//void asyncEvent(Window* pWnd, dword wMsg);

			void ircNick(Text nick) noexcept;
			void ircReal(Text real) noexcept;
			void ircCap(Text caps) noexcept;
			void ircPass(Text pass) noexcept;
			void ircPrivMsg(Channel* channel, Text str) noexcept;
			void ircJoin(Text channel) noexcept;
			void ircPart(Channel* channel) noexcept;
			void ircUser(Text nick, Text real) noexcept;
			bool ircMode(Channel* channel, User* user, char mode, char sign) noexcept;
			void ircKick(Channel* channel, User* user) noexcept;
			void ircQuit(Text str = nullptr) noexcept;
			User* ircFindUser(Text nick) noexcept;
			Channel* ircFindChannel(Text chname) noexcept;

			virtual void onUserCreated(User * user) noexcept;
			virtual void onUserDeleted(User * user) noexcept;
			virtual void onIRCConnect() noexcept;
			virtual void onLine(Text line) noexcept;
			virtual void onNotice(int num, Text message) noexcept;
			virtual void onMessage(User * user, Channel * channel, Text message) noexcept;
			virtual void onJoin(InUser * user) noexcept;
			virtual void onOut(Channel * channel, User * user) noexcept;
			virtual void onNick(User * user, Text nick) noexcept;
			virtual void onJoinOper(InUser * user) noexcept;
			virtual void onUnprocessed(User * user, View<Text> messages) noexcept;

			Text getNick() noexcept;

			ChannelList& getChannelList() noexcept;

			UserList& getUserList() noexcept;

			void onError(Text name, int code) noexcept override;
			void onConnect() noexcept override;
			void onRead() noexcept override;
			void onClose() noexcept override;
			
			ReferenceMap<Text, Text> tags;

		protected:
			BText<1024> m_data;
			UserList m_users;
			ChannelList m_channels;

			void _deleteInUser(Channel* channel, InUser * inuser) noexcept;
			void _quit(User * user) noexcept;
		};


		template <typename C>
		ParameterT<C>::ParameterT(Text str) noexcept
		{
			for (;;)
			{
				if (*str == ':')
				{
					str++;
					push(str);
					return;
				}
				Text param = str.readwith(' ');
				if (param == nullptr)
				{
					push(str);
					return;
				}
				push(param);
			}
		}

		namespace _pri_
		{
			template <typename T>
			T rawToIrc(T value) throws(NotEnoughSpaceException)
			{
				if (T(-1) < 0)
				{
					if (value < 0)
					{
						if (value < -(T)IRCVALUE_MAX(T))
							throw NotEnoughSpaceException();
						return value + IRCVALUE_LIMIT(T);
					}
					else
					{
						if (value >(T)IRCVALUE_MAX(T))
							throw NotEnoughSpaceException();
						return value;
					}
				}
				else
				{
					if (value >= (T)IRCVALUE_LIMIT(T))
						throw NotEnoughSpaceException();
					return value;
				}
			}
			template <typename T>
			T ircToRaw(T value) throws(NotEnoughSpaceException)
			{
				if (T(-1)<0 && (dword)value > IRCVALUE_MAX(T))
					return value - IRCVALUE_LIMIT(T);
				return value;
			}
			template <>
			inline float rawToIrc<float>(float value) throws(NotEnoughSpaceException)
			{
				if (value < 0)
					throw NotEnoughSpaceException();
				return value;
			}
			template <>
			inline float ircToRaw<float>(float value) throws(NotEnoughSpaceException)
			{
				return value;
			}
		}
		template <typename T>
		T rawToIrc(T value) throws(NotEnoughSpaceException)
		{
			using uint_t = uint_sz_t<sizeof(T)>;

			union
			{
				T realValueTemp;
				uint_t temp;
			};
			realValueTemp = _pri_::rawToIrc(value);
			uint_t result = 0;
			for (size_t i = 0; i < sizeof(T); i++)
			{
				uint_t left = temp % 253;
				temp /= 253;
				left++;
				if (left >= 0x0a)
					left++;
				if (left >= 0x0d)
					left++;

				result <<= 8;
				result |= left;
			}
			return (T&)result;
		}
		template <typename T>
		T ircToRaw(T value) throws(NotEnoughSpaceException)
		{
			using uint_t = uint_sz_t<sizeof(T)>;

			uint_t result = 0;
			byte(*values)[sizeof(T)] = (byte(*)[sizeof(T)]) &value;

			for (byte v : values)
			{
				result *= 253;

				if (v >= 0x0d)
					v--;
				if (v >= 0x0a)
					v--;
				v--;
				result += v;
			}
			return _pri_::ircToRaw((T&)result);
		}

	}
}
