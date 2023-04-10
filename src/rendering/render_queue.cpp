#include "render_queue.h"

#include <profiling/scoped_event.h>

#include "texture_binding_cache.h"
#include "draw_call_tree.h"

using namespace rendering;

void RenderQueue::execute()
{
    SCOPED_EVENT("RenderQueue - execute");
    RenderQueueStats stats;

    drain_queues();

    const DrawCallTree tree(std::move(_draw_calls));
    tree.execute(stats);

    // TODO: for some reason the texture binding cache breaks after pause if you don't clear it
    TextureBindingCache::get().unbind_all();
    _queue_stats = stats;
}

void RenderQueue::enqueue_draw(DrawCall&& draw_call)
{
    _draw_call_queue.enqueue(draw_call);
}

const RenderQueueStats& RenderQueue::last_frame_stats() const noexcept
{
    return _queue_stats;
}

void RenderQueue::drain_queues()
{
    DrawCall draw_call;
    while (_draw_call_queue.try_dequeue(draw_call))
    {
        _draw_calls.push_back(draw_call);
    }
}

