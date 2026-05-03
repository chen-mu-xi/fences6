using System;
using System.Collections.Concurrent;

namespace Fences6.App.Services
{
    /// <summary>
    /// 简单的事件消息代理，用于应用内各模块间的松耦合通信
    /// </summary>
    public class EventBroker
    {
        private static readonly Lazy<EventBroker> _instance = new(() => new EventBroker());
        private readonly ConcurrentDictionary<string, Action> _subscribers = new();

        public static EventBroker Instance => _instance.Value;

        private EventBroker() { }

        /// <summary>
        /// 订阅消息
        /// </summary>
        public void Subscribe(string topic, Action action)
        {
            _subscribers.AddOrUpdate(topic, action, (_, existing) => existing + action);
        }

        /// <summary>
        /// 发布消息
        /// </summary>
        public void Publish(string topic)
        {
            if (_subscribers.TryGetValue(topic, out var action))
            {
                action?.Invoke();
            }
        }

        /// <summary>
        /// 取消订阅
        /// </summary>
        public void Unsubscribe(string topic, Action action)
        {
            if (_subscribers.TryGetValue(topic, out var existing))
            {
                var delegateList = existing - action;
                _subscribers.AddOrUpdate(topic, delegateList, (_, _) => delegateList);
            }
        }

        /// <summary>
        /// 清空指定主题的所有订阅
        /// </summary>
        public void Clear(string topic)
        {
            _subscribers.TryRemove(topic, out _);
        }
    }
}
