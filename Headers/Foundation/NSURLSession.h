#ifndef __NSURLSession_h_GNUSTEP_BASE_INCLUDE
#define __NSURLSession_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSHTTPCookieStorage.h>

#if GS_HAVE_NSURLSESSION
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)
@protocol NSURLSessionDelegate;
@protocol NSURLSessionTaskDelegate;

@class GSMultiHandle;
@class GSURLSessionTaskBody;
@class NSError;
@class NSHTTPURLResponse;
@class NSOperationQueue;
@class NSURL;
@class NSURLAuthenticationChallenge;
@class NSURLCache;
@class NSURLCredential;
@class NSURLCredentialStorage;
@class NSURLRequest;
@class NSURLResponse;
@class NSURLSessionConfiguration;
@class NSURLSessionDataTask;
@class NSURLSessionDownloadTask;


/**
 * NSURLSession is a replacement API for NSURLConnection.  It provides
 * options that affect the policy of, and various aspects of the
 * mechanism by which NSURLRequest objects are retrieved from the
 * network.<br />
 *
 * An NSURLSession may be bound to a delegate object.  The delegate is
 * invoked for certain events during the lifetime of a session.
 * 
 * NSURLSession instances are threadsafe.
 *
 * An NSURLSession creates NSURLSessionTask objects which represent the
 * action of a resource being loaded.
 * 
 * NSURLSessionTask objects are always created in a suspended state and
 * must be sent the -resume message before they will execute.
 *
 * Subclasses of NSURLSessionTask are used to syntactically
 * differentiate between data and file downloads.
 * 
 * An NSURLSessionDataTask receives the resource as a series of calls to
 * the URLSession:dataTask:didReceiveData: delegate method.  This is type of
 * task most commonly associated with retrieving objects for immediate parsing
 * by the consumer.
 */
GS_EXPORT_CLASS
@interface NSURLSession : NSObject
{
  NSOperationQueue           *_delegateQueue;
  id <NSURLSessionDelegate>  _delegate;
  NSURLSessionConfiguration  *_configuration;
  NSString                   *_sessionDescription;
  GSMultiHandle              *_multiHandle;
}

/*
 * Customization of NSURLSession occurs during creation of a new session.
 * If you do specify a delegate, the delegate will be retained until after
 * the delegate has been sent the URLSession:didBecomeInvalidWithError: message.
 */
+ (NSURLSession*) sessionWithConfiguration: (NSURLSessionConfiguration*)configuration 
                                  delegate: (id <NSURLSessionDelegate>)delegate 
                             delegateQueue: (NSOperationQueue*)queue;

- (NSOperationQueue*) delegateQueue;

- (id <NSURLSessionDelegate>) delegate;

- (NSURLSessionConfiguration*) configuration;

- (NSString*) sessionDescription;

- (void) setSessionDescription: (NSString*)sessionDescription;

/* -finishTasksAndInvalidate returns immediately and existing tasks will be 
 * allowed to run to completion.  New tasks may not be created.  The session
 * will continue to make delegate callbacks until 
 * URLSession:didBecomeInvalidWithError: has been issued. 
 *
 * When invalidating a background session, it is not safe to create another 
 * background session with the same identifier until 
 * URLSession:didBecomeInvalidWithError: has been issued.
 */
- (void) finishTasksAndInvalidate;

/* -invalidateAndCancel acts as -finishTasksAndInvalidate, but issues
 * -cancel to all outstanding tasks for this session.  Note task 
 * cancellation is subject to the state of the task, and some tasks may
 * have already have completed at the time they are sent -cancel. 
 */
- (void) invalidateAndCancel;

/* 
 * NSURLSessionTask objects are always created in a suspended state and
 * must be sent the -resume message before they will execute.
 */

/* Creates a data task with the given request. 
 * The request may have a body stream. */
- (NSURLSessionDataTask*) dataTaskWithRequest: (NSURLRequest*)request;

/* Creates a data task to retrieve the contents of the given URL. */
- (NSURLSessionDataTask*) dataTaskWithURL: (NSURL*)url;

/* Creates a download task with the given request. */
- (NSURLSessionDownloadTask *) downloadTaskWithRequest: (NSURLRequest *)request;

/* Creates a download task to download the contents of the given URL. */
- (NSURLSessionDownloadTask *) downloadTaskWithURL: (NSURL *)url;

@end

typedef NS_ENUM(NSUInteger, NSURLSessionTaskState) {
  /* The task is currently being serviced by the session */
  NSURLSessionTaskStateRunning = 0,    
  NSURLSessionTaskStateSuspended = 1,
  /* The task has been told to cancel.  
   * The session will receive URLSession:task:didCompleteWithError:. */
  NSURLSessionTaskStateCanceling = 2,  
  /* The task has completed and the session will receive no more 
   * delegate notifications */
  NSURLSessionTaskStateCompleted = 3,  
};

/*
 * NSURLSessionTask - a cancelable object that refers to the lifetime
 * of processing a given request.
 */
GS_EXPORT_CLASS
@interface NSURLSessionTask : NSObject <NSCopying>
{
  /** An identifier for this task, assigned by and unique
   * to the owning session
   */
  NSUInteger    _taskIdentifier;

  /** The request this task was created to handle.
   */
  NSURLRequest  *_originalRequest;

  /** The request this task is currently handling.  This may differ from 
   * originalRequest due to http server redirection
   */
  NSURLRequest  *_currentRequest;

  /** The response to the current request, which may be nil if no response
   * has been received
   */
  NSURLResponse *_response;

  /** number of body bytes already received
   */
  int64_t       _countOfBytesReceived;

  /** number of body bytes already sent
   */
  int64_t       _countOfBytesSent;

  /** number of body bytes we expect to send, derived from 
   * the Content-Length of the HTTP request
   */
  int64_t       _countOfBytesExpectedToSend;

  /** number of byte bytes we expect to receive, usually derived from the 
   * Content-Length header of an HTTP response.
   */
  int64_t       _countOfBytesExpectedToReceive;

  /** a description of the current task for diagnostic purposes
   */
  NSString              *_taskDescription;

  /** The current state of the task within the session.
   */
  NSURLSessionTaskState _state;

  /** The error, if any, delivered via -URLSession:task:didCompleteWithError:
   * This is nil until an error has occured.
   */
  NSError               *_error;

  /** The dispatch queue used to handle this request/response.
   * This is actualy a libdispatch queue of type dispatch_queue_t, but on all
   * known implementations this is a pointer, so void* is the correct size.
   */
  void                  *_workQueue;

  NSUInteger            _suspendCount;

  GSURLSessionTaskBody  *_knownBody;
}

- (NSUInteger) taskIdentifier;

- (NSURLRequest*) originalRequest;

- (NSURLRequest*) currentRequest;

- (NSURLResponse*) response;
- (void) setResponse: (NSURLResponse*)response;

- (int64_t) countOfBytesReceived;

- (int64_t) countOfBytesSent;

- (int64_t) countOfBytesExpectedToSend;

- (int64_t) countOfBytesExpectedToReceive;

- (NSString*) taskDescription;

- (void) setTaskDescription: (NSString*)taskDescription;

- (NSURLSessionTaskState) state;

- (NSError*) error;

- (NSURLSession*) session;

/* -cancel returns immediately, but marks a task as being canceled.
 * The task will signal -URLSession:task:didCompleteWithError: with an
 * error value of { NSURLErrorDomain, NSURLErrorCancelled }. In some 
 * cases, the task may signal other work before it acknowledges the 
 * cancelation.  -cancel may be sent to a task that has been suspended.
 */
- (void) cancel;

/*
 * Suspending a task will prevent the NSURLSession from continuing to
 * load data.  There may still be delegate calls made on behalf of
 * this task (for instance, to report data received while suspending)
 * but no further transmissions will be made on behalf of the task
 * until -resume is sent.  The timeout timer associated with the task
 * will be disabled while a task is suspended.
 */
- (void) suspend;
- (void) resume;

@end

GS_EXPORT_CLASS
@interface NSURLSessionDataTask : NSURLSessionTask
@end

GS_EXPORT_CLASS
@interface NSURLSessionUploadTask : NSURLSessionDataTask
@end

GS_EXPORT_CLASS
@interface NSURLSessionDownloadTask : NSURLSessionTask
@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_11,GS_API_LATEST)
GS_EXPORT_CLASS
@interface NSURLSessionStreamTask : NSURLSessionTask
@end
#endif

/*
 * Configuration options for an NSURLSession.  When a session is
 * created, a copy of the configuration object is made - you cannot
 * modify the configuration of a session after it has been created.
 */
GS_EXPORT_CLASS
@interface NSURLSessionConfiguration : NSObject <NSCopying>
{
  NSURLCache               *_URLCache;
  NSURLRequestCachePolicy  _requestCachePolicy;
  NSArray                  *_protocolClasses;
  NSInteger                _HTTPMaximumConnectionLifetime;
  NSInteger                _HTTPMaximumConnectionsPerHost;
  BOOL                     _HTTPShouldUsePipelining;
  NSHTTPCookieAcceptPolicy _HTTPCookieAcceptPolicy;
  NSHTTPCookieStorage      *_HTTPCookieStorage;
  NSURLCredentialStorage   *_URLCredentialStorage;
  BOOL                     _HTTPShouldSetCookies;
  NSDictionary             *_HTTPAdditionalHeaders;
}

- (NSURLRequest*) configureRequest: (NSURLRequest*)request;

@property (class, readonly, strong)
  NSURLSessionConfiguration *defaultSessionConfiguration;

- (NSDictionary*) HTTPAdditionalHeaders;

- (NSHTTPCookieAcceptPolicy) HTTPCookieAcceptPolicy;

- (NSHTTPCookieStorage*) HTTPCookieStorage;

#if     !NO_GNUSTEP 
- (NSInteger) HTTPMaximumConnectionLifetime;
#endif

- (NSInteger) HTTPMaximumConnectionsPerHost;

- (BOOL) HTTPShouldSetCookies;

- (BOOL) HTTPShouldUsePipelining;

- (NSArray*) protocolClasses;

- (NSURLRequestCachePolicy) requestCachePolicy;

- (void) setHTTPAdditionalHeaders: (NSDictionary*)headers;

- (void) setHTTPCookieAcceptPolicy: (NSHTTPCookieAcceptPolicy)policy;

- (void) setHTTPCookieStorage: (NSHTTPCookieStorage*)storage;

#if     !NO_GNUSTEP 
/** Permits a session to be configured so that older connections are reused.
 * A value of zero or less uses the default behavior where connections are
 * reused as long as they are not older than 118 seconds, which is reasonable
 * for the vast majority if situations.
 */
- (void) setHTTPMaximumConnectionLifetime: (NSInteger)n;
#endif

- (void) setHTTPMaximumConnectionsPerHost: (NSInteger)n;

- (void) setHTTPShouldSetCookies: (BOOL)flag;

- (void) setHTTPShouldUsePipelining: (BOOL)flag;

- (void) setRequestCachePolicy: (NSURLRequestCachePolicy)policy;

- (void) setURLCache: (NSURLCache*)cache;

- (void) setURLCredentialStorage: (NSURLCredentialStorage*)storage;

- (NSURLCache*) URLCache;

- (NSURLCredentialStorage*) URLCredentialStorage;

@end

typedef NS_ENUM(NSInteger, NSURLSessionAuthChallengeDisposition) {
  NSURLSessionAuthChallengeUseCredential = 0,
  NSURLSessionAuthChallengePerformDefaultHandling = 1,
  NSURLSessionAuthChallengeCancelAuthenticationChallenge = 2,
  NSURLSessionAuthChallengeRejectProtectionSpace = 3
};

typedef NS_ENUM(NSInteger, NSURLSessionResponseDisposition) {
  NSURLSessionResponseCancel = 0,
  NSURLSessionResponseAllow = 1,
  NSURLSessionResponseBecomeDownload = 2,
  NSURLSessionResponseBecomeStream  = 3
};

@protocol NSURLSessionDelegate <NSObject>
@optional
/* The last message a session receives.  A session will only become
 * invalid because of a systemic error or when it has been
 * explicitly invalidated, in which case the error parameter will be nil.
 */
- (void)         URLSession: (NSURLSession*)session 
  didBecomeInvalidWithError: (NSError*)error;

/* Implementing this method permits a delegate to provide authentication
 * credentials in response to a challenge from the remote server.
 */
- (void) URLSession: (NSURLSession*)session
didReceiveChallenge: (NSURLAuthenticationChallenge*)challenge
  completionHandler: (void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential *credential))handler;

@end

@protocol NSURLSessionTaskDelegate <NSURLSessionDelegate>
@optional
/* Sent as the last message related to a specific task.  Error may be
 * nil, which implies that no error occurred and this task is complete. 
 */
- (void )   URLSession: (NSURLSession*)session 
                  task: (NSURLSessionTask*)task
  didCompleteWithError: (NSError*)error;
     
/* Called to request authentication credentials from the delegate when
 * an authentication request is received from the server which is specific
 * to this task.
 */
- (void) URLSession: (NSURLSession*)session 
	       task: (NSURLSessionTask*)task 
didReceiveChallenge: (NSURLAuthenticationChallenge*)challenge 
  completionHandler: (void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential *credential))handler;

/* Periodically informs the delegate of the progress of sending body content 
 * to the server.
 */
- (void)       URLSession: (NSURLSession*)session 
                     task: (NSURLSessionTask*)task 
          didSendBodyData: (int64_t)bytesSent 
           totalBytesSent: (int64_t)totalBytesSent 
 totalBytesExpectedToSend: (int64_t)totalBytesExpectedToSend;

/* An HTTP request is attempting to perform a redirection to a different
 * URL. You must invoke the completion routine to allow the
 * redirection, allow the redirection with a modified request, or
 * pass nil to the completionHandler to cause the body of the redirection 
 * response to be delivered as the payload of this request. The default
 * is to follow redirections. 
 *
 */
- (void)          URLSession: (NSURLSession*)session 
                        task: (NSURLSessionTask*)task
  willPerformHTTPRedirection: (NSHTTPURLResponse*)response
                  newRequest: (NSURLRequest*)request
           completionHandler: (void (^)(NSURLRequest*))completionHandler;

@end

@protocol NSURLSessionDataDelegate <NSURLSessionTaskDelegate>
@optional
/* Sent when data is available for the delegate to consume.
 */
- (void) URLSession: (NSURLSession*)session 
           dataTask: (NSURLSessionDataTask*)dataTask
     didReceiveData: (NSData*)data;

/** Informs the delegate of a response.  This message is sent when all the
 * response headers have arrived, before the body of the response arrives.
 */
- (void) URLSession: (NSURLSession*)session
           dataTask: (NSURLSessionDataTask*)dataTask
 didReceiveResponse: (NSURLResponse*)response
  completionHandler: (void (^)(NSURLSessionResponseDisposition disposition))completionHandler;

@end

@protocol NSURLSessionDownloadDelegate <NSURLSessionTaskDelegate>

/* Sent when a download task that has completed a download.  The delegate should 
 * copy or move the file at the given location to a new location as it will be 
 * removed when the delegate message returns. URLSession:task:didCompleteWithError: will
 * still be called.
 */
- (void)         URLSession: (NSURLSession *)session
               downloadTask: (NSURLSessionDownloadTask *)downloadTask
  didFinishDownloadingToURL: (NSURL *)location;

@optional
/* Sent periodically to notify the delegate of download progress. */
- (void)         URLSession: (NSURLSession *)session
               downloadTask: (NSURLSessionDownloadTask *)downloadTask
               didWriteData: (int64_t)bytesWritten
          totalBytesWritten: (int64_t)totalBytesWritten
  totalBytesExpectedToWrite: (int64_t)totalBytesExpectedToWrite;

/* Sent when a download has been resumed. If a download failed with an
 * error, the -userInfo dictionary of the error will contain an
 * NSURLSessionDownloadTaskResumeData key, whose value is the resume
 * data. 
 */
- (void)  URLSession: (NSURLSession *)session
        downloadTask: (NSURLSessionDownloadTask *)downloadTask
   didResumeAtOffset: (int64_t)fileOffset
  expectedTotalBytes: (int64_t)expectedTotalBytes;

@end

#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_11,GS_API_LATEST)

/**
 * An NSURLSessionStreamTask provides an interface to perform reads
 * and writes to a TCP/IP stream created via NSURLSession.  This task
 * may be explicitly created from an NSURLSession, or created as a
 * result of the appropriate disposition response to a
 * -URLSession:dataTask:didReceiveResponse: delegate message.
 * 
 * NSURLSessionStreamTask can be used to perform asynchronous reads
 * and writes.  Reads and writes are enqueued and executed serially,
 * with the completion handler being invoked on the sessions delegate
 * queue.  If an error occurs, or the task is canceled, all
 * outstanding read and write calls will have their completion
 * handlers invoked with an appropriate error.
 *
 * It is also possible to create NSInputStream and NSOutputStream
 * instances from an NSURLSessionTask by sending
 * -captureStreams to the task.  All outstanding read and writess are
 * completed before the streams are created.  Once the streams are
 * delivered to the session delegate, the task is considered complete
 * and will receive no more messsages.  These streams are
 * disassociated from the underlying session.
 */
@interface NSURLSessionStreamTask : NSURLSessionTask

/** 
 * Read minBytes, or at most maxBytes bytes and invoke the completion
 * handler on the sessions delegate queue with the data or an error.
 * If an error occurs, any outstanding reads will also fail, and new
 * read requests will error out immediately.
 */
- (void) readDataOfMinLength: (NSUInteger)minBytes
                   maxLength: (NSUInteger)maxBytes
                     timeout: (NSTimeInterval)timeout
           completionHandler: (void (^) (NSData *data, BOOL atEOF, NSError *error))completionHandler;

/**
 * Write the data completely to the underlying socket.  If all the
 * bytes have not been written by the timeout, a timeout error will
 * occur.  Note that invocation of the completion handler does not
 * guarantee that the remote side has received all the bytes, only
 * that they have been written to the kernel. */
- (void)         writeData: (NSData *)data
                   timeout: (NSTimeInterval)timeout
         completionHandler: (void (^) (NSError *error))completionHandler;

/**
 * -captureStreams completes any already enqueued reads
 * and writes, and then invokes the
 * URLSession:streamTask:didBecomeInputStream:outputStream: delegate
 * message. When that message is received, the task object is
 * considered completed and will not receive any more delegate
 * messages. */
- (void) captureStreams;

/**
 * Enqueue a request to close the write end of the underlying socket.
 * All outstanding IO will complete before the write side of the
 * socket is closed.  The server, however, may continue to write bytes
 * back to the client, so best practice is to continue reading from
 * the server until you receive EOF.
 */
- (void) closeWrite;

/**
 * Enqueue a request to close the read side of the underlying socket.
 * All outstanding IO will complete before the read side is closed.
 * You may continue writing to the server.
 */
- (void) closeRead;

/**
 * Begin encrypted handshake.  The handshake begins after all pending 
 * IO has completed.  TLS authentication callbacks are sent to the 
 * session's -URLSession:task:didReceiveChallenge:completionHandler:
 */
- (void) startSecureConnection;

@end

@protocol NSURLSessionStreamDelegate <NSURLSessionTaskDelegate>
@optional

/**
 * Indicates that the read side of a connection has been closed.  Any
 * outstanding reads complete, but future reads will immediately fail.
 * This may be sent even when no reads are in progress. However, when
 * this delegate message is received, there may still be bytes
 * available.  You only know that no more bytes are available when you
 * are able to read until EOF. */
- (void)              URLSession: (NSURLSession *)session
         readClosedForStreamTask: (NSURLSessionStreamTask *)streamTask;

/**
 * Indicates that the write side of a connection has been closed.
 * Any outstanding writes complete, but future writes will immediately
 * fail.
 */
- (void)               URLSession: (NSURLSession *)session
         writeClosedForStreamTask: (NSURLSessionStreamTask *)streamTask;

/**
 * A notification that the system has determined that a better route
 * to the host has been detected (eg, a wi-fi interface becoming
 * available.)  This is a hint to the delegate that it may be
 * desirable to create a new task for subsequent work.  Note that
 * there is no guarantee that the future task will be able to connect
 * to the host, so callers should should be prepared for failure of
 * reads and writes over any new interface. */
- (void)                         URLSession: (NSURLSession *)session
         betterRouteDiscoveredForStreamTask: (NSURLSessionStreamTask *)streamTask;

/**
 * The given task has been completed, and unopened NSInputStream and
 * NSOutputStream objects are created from the underlying network
 * connection.  This will only be invoked after all enqueued IO has
 * completed (including any necessary handshakes.)  The streamTask
 * will not receive any further delegate messages.
 */
- (void)           URLSession: (NSURLSession *)session
                   streamTask: (NSURLSessionStreamTask *)streamTask
         didBecomeInputStream: (NSInputStream *)inputStream
                 outputStream: (NSOutputStream *)outputStream;

@end

#endif
#endif
#endif