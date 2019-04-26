#pragma once
#include <KR3/main.h>
#include <KRUtil/parser/jsonparser.h>
namespace test
{
	namespace test
	{
		struct __format_base
		{
			struct error_t
			{
				struct errors_t
				{
					kr::AText domain;
					kr::AText reason;
					kr::AText message;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("domain", &domain);
							field("reason", &reason);
							field("message", &message);
						}
						);
					}
				};
				Array<errors_t> errors;
				uint code = 0;
				kr::AText message;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("errors", &errors);
						field("code", &code);
						field("message", &message);
					}
					);
				}
			};
			Keep<error_t> error;
			void parseJson(JsonParser & parser)
			{
				parser.fields([this](JsonField& field)
				{
					field("error", &error);
				}
				);
			}
		};
		namespace liveBroadcast
		{
			struct Resource : __format_base
			{
				/** youtube#liveBroadcast */ kr::AText kind;
				/** etag */ kr::AText etag;
				kr::AText id;
				struct snippet_t
				{
					/** datetime */ kr::AText publishedAt;
					kr::AText channelId;
					kr::AText title;
					kr::AText description;
					struct thumbnails_t_t
					{
						kr::AText url;
						uint width = 0;
						uint height = 0;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("url", &url);
								field("width", &width);
								field("height", &height);
							}
							);
						}
					};
					using thumbnails_t = Map<Text, thumbnails_t_t>;
					thumbnails_t thumbnails;
					/** datetime */ kr::AText scheduledStartTime;
					/** datetime */ kr::AText scheduledEndTime;
					/** datetime */ kr::AText actualStartTime;
					/** datetime */ kr::AText actualEndTime;
					bool isDefaultBroadcast = false;
					kr::AText liveChatId;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("publishedAt", &publishedAt);
							field("channelId", &channelId);
							field("title", &title);
							field("description", &description);
							field("thumbnails", &thumbnails);
							field("scheduledStartTime", &scheduledStartTime);
							field("scheduledEndTime", &scheduledEndTime);
							field("actualStartTime", &actualStartTime);
							field("actualEndTime", &actualEndTime);
							field("isDefaultBroadcast", &isDefaultBroadcast);
							field("liveChatId", &liveChatId);
						}
						);
					}
				};
				snippet_t snippet;
				struct status_t
				{
					kr::AText lifeCycleStatus;
					kr::AText privacyStatus;
					kr::AText recordingStatus;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("lifeCycleStatus", &lifeCycleStatus);
							field("privacyStatus", &privacyStatus);
							field("recordingStatus", &recordingStatus);
						}
						);
					}
				};
				status_t status;
				struct contentDetails_t
				{
					kr::AText boundStreamId;
					/** datetime */ kr::AText boundStreamLastUpdateTimeMs;
					struct monitorStream_t
					{
						bool enableMonitorStream = false;
						uint broadcastStreamDelayMs = 0;
						kr::AText embedHtml;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("enableMonitorStream", &enableMonitorStream);
								field("broadcastStreamDelayMs", &broadcastStreamDelayMs);
								field("embedHtml", &embedHtml);
							}
							);
						}
					};
					monitorStream_t monitorStream;
					bool enableEmbed = false;
					bool enableDvr = false;
					bool enableContentEncryption = false;
					bool startWithSlate = false;
					bool recordFromStart = false;
					bool enableClosedCaptions = false;
					kr::AText closedCaptionsType;
					kr::AText projection;
					bool enableLowLatency = false;
					bool latencyPreference = false;
					bool enableAutoStart = false;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("boundStreamId", &boundStreamId);
							field("boundStreamLastUpdateTimeMs", &boundStreamLastUpdateTimeMs);
							field("monitorStream", &monitorStream);
							field("enableEmbed", &enableEmbed);
							field("enableDvr", &enableDvr);
							field("enableContentEncryption", &enableContentEncryption);
							field("startWithSlate", &startWithSlate);
							field("recordFromStart", &recordFromStart);
							field("enableClosedCaptions", &enableClosedCaptions);
							field("closedCaptionsType", &closedCaptionsType);
							field("projection", &projection);
							field("enableLowLatency", &enableLowLatency);
							field("latencyPreference", &latencyPreference);
							field("enableAutoStart", &enableAutoStart);
						}
						);
					}
				};
				contentDetails_t contentDetails;
				struct statistics_t
				{
					uint64_t totalChatCount = 0;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("totalChatCount", &totalChatCount);
						}
						);
					}
				};
				statistics_t statistics;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("kind", &kind);
						field("etag", &etag);
						field("id", &id);
						field("snippet", &snippet);
						field("status", &status);
						field("contentDetails", &contentDetails);
						field("statistics", &statistics);
					}
					);
				}
			};
			struct List : __format_base
			{
				/** youtube#liveBroadcastListResponse */ kr::AText kind;
				/** etag */ kr::AText etag;
				kr::AText nextPageToken;
				kr::AText prevPageToken;
				struct pageInfo_t
				{
					int totalResults = 0;
					int resultsPerPage = 0;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("totalResults", &totalResults);
							field("resultsPerPage", &resultsPerPage);
						}
						);
					}
				};
				pageInfo_t pageInfo;
				Array<liveBroadcast::Resource> items;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("kind", &kind);
						field("etag", &etag);
						field("nextPageToken", &nextPageToken);
						field("prevPageToken", &prevPageToken);
						field("pageInfo", &pageInfo);
						field("items", &items);
					}
					);
				}
			};
		}
		namespace liveChatMessage
		{
			struct Resource : __format_base
			{
				/** youtube#liveChatMessage */ kr::AText kind;
				/** etag */ kr::AText etag;
				kr::AText id;
				struct snippet_t
				{
					kr::AText type;
					kr::AText liveChatId;
					kr::AText authorChannelId;
					/** datetime */ kr::AText publishedAt;
					bool hasDisplayContent = false;
					kr::AText displayMessage;
					struct fanFundingEventDetails_t
					{
						uint64_t amountMicros = 0;
						kr::AText currency;
						kr::AText amountDisplayString;
						kr::AText userComment;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("amountMicros", &amountMicros);
								field("currency", &currency);
								field("amountDisplayString", &amountDisplayString);
								field("userComment", &userComment);
							}
							);
						}
					};
					fanFundingEventDetails_t fanFundingEventDetails;
					struct textMessageDetails_t
					{
						kr::AText messageText;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("messageText", &messageText);
							}
							);
						}
					};
					textMessageDetails_t textMessageDetails;
					struct messageDeletedDetails_t
					{
						kr::AText deletedMessageId;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("deletedMessageId", &deletedMessageId);
							}
							);
						}
					};
					messageDeletedDetails_t messageDeletedDetails;
					struct userBannedDetails_t
					{
						struct bannedUserDetails_t
						{
							kr::AText channelId;
							kr::AText channelUrl;
							kr::AText displayName;
							kr::AText profileImageUrl;
							void parseJson(JsonParser & parser)
							{
								parser.fields([this](JsonField& field)
								{
									field("channelId", &channelId);
									field("channelUrl", &channelUrl);
									field("displayName", &displayName);
									field("profileImageUrl", &profileImageUrl);
								}
								);
							}
						};
						bannedUserDetails_t bannedUserDetails;
						kr::AText banType;
						uint64_t banDurationSeconds = 0;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("bannedUserDetails", &bannedUserDetails);
								field("banType", &banType);
								field("banDurationSeconds", &banDurationSeconds);
							}
							);
						}
					};
					userBannedDetails_t userBannedDetails;
					struct superChatDetails_t
					{
						uint64_t amountMicros = 0;
						kr::AText currency;
						kr::AText amountDisplayString;
						kr::AText userComment;
						uint tier = 0;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("amountMicros", &amountMicros);
								field("currency", &currency);
								field("amountDisplayString", &amountDisplayString);
								field("userComment", &userComment);
								field("tier", &tier);
							}
							);
						}
					};
					superChatDetails_t superChatDetails;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("type", &type);
							field("liveChatId", &liveChatId);
							field("authorChannelId", &authorChannelId);
							field("publishedAt", &publishedAt);
							field("hasDisplayContent", &hasDisplayContent);
							field("displayMessage", &displayMessage);
							field("fanFundingEventDetails", &fanFundingEventDetails);
							field("textMessageDetails", &textMessageDetails);
							field("messageDeletedDetails", &messageDeletedDetails);
							field("userBannedDetails", &userBannedDetails);
							field("superChatDetails", &superChatDetails);
						}
						);
					}
				};
				snippet_t snippet;
				struct authorDetails_t
				{
					kr::AText channelId;
					kr::AText channelUrl;
					kr::AText displayName;
					kr::AText profileImageUrl;
					bool isVerified = false;
					bool isChatOwner = false;
					bool isChatSponsor = false;
					bool isChatModerator = false;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("channelId", &channelId);
							field("channelUrl", &channelUrl);
							field("displayName", &displayName);
							field("profileImageUrl", &profileImageUrl);
							field("isVerified", &isVerified);
							field("isChatOwner", &isChatOwner);
							field("isChatSponsor", &isChatSponsor);
							field("isChatModerator", &isChatModerator);
						}
						);
					}
				};
				authorDetails_t authorDetails;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("kind", &kind);
						field("etag", &etag);
						field("id", &id);
						field("snippet", &snippet);
						field("authorDetails", &authorDetails);
					}
					);
				}
			};
			struct List : __format_base
			{
				/** youtube#liveChatMessageListResponse */ kr::AText kind;
				/** etag */ kr::AText etag;
				kr::AText nextPageToken;
				uint pollingIntervalMillis = 0;
				/** datetime */ kr::AText offlineAt;
				struct pageInfo_t
				{
					int totalResults = 0;
					int resultsPerPage = 0;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("totalResults", &totalResults);
							field("resultsPerPage", &resultsPerPage);
						}
						);
					}
				};
				pageInfo_t pageInfo;
				Array<liveChatMessage::Resource> items;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("kind", &kind);
						field("etag", &etag);
						field("nextPageToken", &nextPageToken);
						field("pollingIntervalMillis", &pollingIntervalMillis);
						field("offlineAt", &offlineAt);
						field("pageInfo", &pageInfo);
						field("items", &items);
					}
					);
				}
			};
		}
		namespace search
		{
			struct Resource : __format_base
			{
				/** youtube#searchResult */ kr::AText kind;
				/** etag */ kr::AText etag;
				struct id_t
				{
					kr::AText kind;
					kr::AText videoId;
					kr::AText channelId;
					kr::AText playlistId;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("kind", &kind);
							field("videoId", &videoId);
							field("channelId", &channelId);
							field("playlistId", &playlistId);
						}
						);
					}
				};
				id_t id;
				struct snippet_t
				{
					/** datetime */ kr::AText publishedAt;
					kr::AText channelId;
					kr::AText title;
					kr::AText description;
					struct thumbnails_t_t
					{
						kr::AText url;
						uint width = 0;
						uint height = 0;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("url", &url);
								field("width", &width);
								field("height", &height);
							}
							);
						}
					};
					using thumbnails_t = Map<Text, thumbnails_t_t>;
					thumbnails_t thumbnails;
					kr::AText channelTitle;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("publishedAt", &publishedAt);
							field("channelId", &channelId);
							field("title", &title);
							field("description", &description);
							field("thumbnails", &thumbnails);
							field("channelTitle", &channelTitle);
						}
						);
					}
				};
				snippet_t snippet;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("kind", &kind);
						field("etag", &etag);
						field("id", &id);
						field("snippet", &snippet);
					}
					);
				}
			};
			struct List : __format_base
			{
				/** youtube#searchListResponse */ kr::AText kind;
				/** etag */ kr::AText etag;
				kr::AText nextPageToken;
				kr::AText prevPageToken;
				struct pageInfo_t
				{
					int totalResults = 0;
					int resultsPerPage = 0;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("totalResults", &totalResults);
							field("resultsPerPage", &resultsPerPage);
						}
						);
					}
				};
				pageInfo_t pageInfo;
				Array<search::Resource> items;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("kind", &kind);
						field("etag", &etag);
						field("nextPageToken", &nextPageToken);
						field("prevPageToken", &prevPageToken);
						field("pageInfo", &pageInfo);
						field("items", &items);
					}
					);
				}
			};
		}
		namespace video
		{
			struct Resource : __format_base
			{
				/** youtube#video */ kr::AText kind;
				/** etag */ kr::AText etag;
				kr::AText id;
				struct snippet_t
				{
					/** datetime */ kr::AText publishedAt;
					kr::AText channelId;
					kr::AText title;
					kr::AText description;
					struct thumbnails_t_t
					{
						kr::AText url;
						uint width = 0;
						uint height = 0;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("url", &url);
								field("width", &width);
								field("height", &height);
							}
							);
						}
					};
					using thumbnails_t = Map<Text, thumbnails_t_t>;
					thumbnails_t thumbnails;
					kr::AText channelTitle;
					Array<kr::AText> tags;
					kr::AText categoryId;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("publishedAt", &publishedAt);
							field("channelId", &channelId);
							field("title", &title);
							field("description", &description);
							field("thumbnails", &thumbnails);
							field("channelTitle", &channelTitle);
							field("tags", &tags);
							field("categoryId", &categoryId);
						}
						);
					}
				};
				snippet_t snippet;
				struct contentDetails_t
				{
					kr::AText duration;
					kr::AText dimension;
					kr::AText definition;
					kr::AText caption;
					bool licensedContent = false;
					struct regionRestriction_t
					{
						Array<kr::AText> allowed;
						Array<kr::AText> blocked;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("allowed", &allowed);
								field("blocked", &blocked);
							}
							);
						}
					};
					regionRestriction_t regionRestriction;
					struct contentRating_t
					{
						kr::AText mpaaRating;
						kr::AText tvpgRating;
						kr::AText bbfcRating;
						kr::AText chvrsRating;
						kr::AText eirinRating;
						kr::AText cbfcRating;
						kr::AText fmocRating;
						kr::AText icaaRating;
						kr::AText acbRating;
						kr::AText oflcRating;
						kr::AText fskRating;
						kr::AText kmrbRating;
						kr::AText djctqRating;
						kr::AText russiaRating;
						kr::AText rtcRating;
						kr::AText ytRating;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("mpaaRating", &mpaaRating);
								field("tvpgRating", &tvpgRating);
								field("bbfcRating", &bbfcRating);
								field("chvrsRating", &chvrsRating);
								field("eirinRating", &eirinRating);
								field("cbfcRating", &cbfcRating);
								field("fmocRating", &fmocRating);
								field("icaaRating", &icaaRating);
								field("acbRating", &acbRating);
								field("oflcRating", &oflcRating);
								field("fskRating", &fskRating);
								field("kmrbRating", &kmrbRating);
								field("djctqRating", &djctqRating);
								field("russiaRating", &russiaRating);
								field("rtcRating", &rtcRating);
								field("ytRating", &ytRating);
							}
							);
						}
					};
					contentRating_t contentRating;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("duration", &duration);
							field("dimension", &dimension);
							field("definition", &definition);
							field("caption", &caption);
							field("licensedContent", &licensedContent);
							field("regionRestriction", &regionRestriction);
							field("contentRating", &contentRating);
						}
						);
					}
				};
				contentDetails_t contentDetails;
				struct status_t
				{
					kr::AText uploadStatus;
					kr::AText failureReason;
					kr::AText rejectionReason;
					kr::AText privacyStatus;
					kr::AText license;
					bool embeddable = false;
					bool publicStatsViewable = false;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("uploadStatus", &uploadStatus);
							field("failureReason", &failureReason);
							field("rejectionReason", &rejectionReason);
							field("privacyStatus", &privacyStatus);
							field("license", &license);
							field("embeddable", &embeddable);
							field("publicStatsViewable", &publicStatsViewable);
						}
						);
					}
				};
				status_t status;
				struct statistics_t
				{
					uint64_t viewCount = 0;
					uint64_t likeCount = 0;
					uint64_t dislikeCount = 0;
					uint64_t favoriteCount = 0;
					uint64_t commentCount = 0;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("viewCount", &viewCount);
							field("likeCount", &likeCount);
							field("dislikeCount", &dislikeCount);
							field("favoriteCount", &favoriteCount);
							field("commentCount", &commentCount);
						}
						);
					}
				};
				statistics_t statistics;
				struct player_t
				{
					kr::AText embedHtml;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("embedHtml", &embedHtml);
						}
						);
					}
				};
				player_t player;
				struct topicDetails_t
				{
					Array<kr::AText> topicIds;
					Array<kr::AText> relevantTopicIds;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("topicIds", &topicIds);
							field("relevantTopicIds", &relevantTopicIds);
						}
						);
					}
				};
				topicDetails_t topicDetails;
				struct recordingDetails_t
				{
					kr::AText locationDescription;
					struct location_t
					{
						double latitude;
						double longitude;
						double altitude;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("latitude", &latitude);
								field("longitude", &longitude);
								field("altitude", &altitude);
							}
							);
						}
					};
					location_t location;
					/** datetime */ kr::AText recordingDate;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("locationDescription", &locationDescription);
							field("location", &location);
							field("recordingDate", &recordingDate);
						}
						);
					}
				};
				recordingDetails_t recordingDetails;
				struct fileDetails_t
				{
					kr::AText fileName;
					uint64_t fileSize = 0;
					kr::AText fileType;
					kr::AText container;
					struct videoStreams_t
					{
						uint widthPixels = 0;
						uint heightPixels = 0;
						double frameRateFps;
						double aspectRatio;
						kr::AText codec;
						uint64_t bitrateBps = 0;
						kr::AText rotation;
						kr::AText vendor;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("widthPixels", &widthPixels);
								field("heightPixels", &heightPixels);
								field("frameRateFps", &frameRateFps);
								field("aspectRatio", &aspectRatio);
								field("codec", &codec);
								field("bitrateBps", &bitrateBps);
								field("rotation", &rotation);
								field("vendor", &vendor);
							}
							);
						}
					};
					Array<videoStreams_t> videoStreams;
					struct audioStreams_t
					{
						uint channelCount = 0;
						kr::AText codec;
						uint64_t bitrateBps = 0;
						kr::AText vendor;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("channelCount", &channelCount);
								field("codec", &codec);
								field("bitrateBps", &bitrateBps);
								field("vendor", &vendor);
							}
							);
						}
					};
					Array<audioStreams_t> audioStreams;
					uint64_t durationMs = 0;
					uint64_t bitrateBps = 0;
					struct recordingLocation_t
					{
						double latitude;
						double longitude;
						double altitude;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("latitude", &latitude);
								field("longitude", &longitude);
								field("altitude", &altitude);
							}
							);
						}
					};
					recordingLocation_t recordingLocation;
					kr::AText creationTime;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("fileName", &fileName);
							field("fileSize", &fileSize);
							field("fileType", &fileType);
							field("container", &container);
							field("videoStreams", &videoStreams);
							field("audioStreams", &audioStreams);
							field("durationMs", &durationMs);
							field("bitrateBps", &bitrateBps);
							field("recordingLocation", &recordingLocation);
							field("creationTime", &creationTime);
						}
						);
					}
				};
				fileDetails_t fileDetails;
				struct processingDetails_t
				{
					kr::AText processingStatus;
					struct processingProgress_t
					{
						uint64_t partsTotal = 0;
						uint64_t partsProcessed = 0;
						uint64_t timeLeftMs = 0;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("partsTotal", &partsTotal);
								field("partsProcessed", &partsProcessed);
								field("timeLeftMs", &timeLeftMs);
							}
							);
						}
					};
					processingProgress_t processingProgress;
					kr::AText processingFailureReason;
					kr::AText fileDetailsAvailability;
					kr::AText processingIssuesAvailability;
					kr::AText tagSuggestionsAvailability;
					kr::AText editorSuggestionsAvailability;
					kr::AText thumbnailsAvailability;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("processingStatus", &processingStatus);
							field("processingProgress", &processingProgress);
							field("processingFailureReason", &processingFailureReason);
							field("fileDetailsAvailability", &fileDetailsAvailability);
							field("processingIssuesAvailability", &processingIssuesAvailability);
							field("tagSuggestionsAvailability", &tagSuggestionsAvailability);
							field("editorSuggestionsAvailability", &editorSuggestionsAvailability);
							field("thumbnailsAvailability", &thumbnailsAvailability);
						}
						);
					}
				};
				processingDetails_t processingDetails;
				struct suggestions_t
				{
					Array<kr::AText> processingErrors;
					Array<kr::AText> processingWarnings;
					Array<kr::AText> processingHints;
					struct tagSuggestions_t
					{
						kr::AText tag;
						Array<kr::AText> categoryRestricts;
						void parseJson(JsonParser & parser)
						{
							parser.fields([this](JsonField& field)
							{
								field("tag", &tag);
								field("categoryRestricts", &categoryRestricts);
							}
							);
						}
					};
					Array<tagSuggestions_t> tagSuggestions;
					Array<kr::AText> editorSuggestions;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("processingErrors", &processingErrors);
							field("processingWarnings", &processingWarnings);
							field("processingHints", &processingHints);
							field("tagSuggestions", &tagSuggestions);
							field("editorSuggestions", &editorSuggestions);
						}
						);
					}
				};
				suggestions_t suggestions;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("kind", &kind);
						field("etag", &etag);
						field("id", &id);
						field("snippet", &snippet);
						field("contentDetails", &contentDetails);
						field("status", &status);
						field("statistics", &statistics);
						field("player", &player);
						field("topicDetails", &topicDetails);
						field("recordingDetails", &recordingDetails);
						field("fileDetails", &fileDetails);
						field("processingDetails", &processingDetails);
						field("suggestions", &suggestions);
					}
					);
				}
			};
			struct List : __format_base
			{
				/** youtube#videoListResponse */ kr::AText kind;
				/** etag */ kr::AText etag;
				kr::AText nextPageToken;
				kr::AText prevPageToken;
				struct pageInfo_t
				{
					int totalResults = 0;
					int resultsPerPage = 0;
					void parseJson(JsonParser & parser)
					{
						parser.fields([this](JsonField& field)
						{
							field("totalResults", &totalResults);
							field("resultsPerPage", &resultsPerPage);
						}
						);
					}
				};
				pageInfo_t pageInfo;
				Array<video::Resource> items;
				void parseJson(JsonParser & parser)
				{
					parser.fields([this](JsonField& field)
					{
						field("kind", &kind);
						field("etag", &etag);
						field("nextPageToken", &nextPageToken);
						field("prevPageToken", &prevPageToken);
						field("pageInfo", &pageInfo);
						field("items", &items);
					}
					);
				}
			};
		}
	}
}
