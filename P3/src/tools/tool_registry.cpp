#include "tools/tool_registry.h"
#include <utility>
#include <stdexcept>
#include <vector>
#include <string>

ToolRegistry::ToolRegistry() {
    size_ = 0;
    buckets_.resize(16);
}

ToolRegistry::~ToolRegistry() {
    for (auto& bucket : buckets_) {
        bucket.clear();
    }
    buckets_.clear();
    size_ = 0;
}

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    // FNV-1a hash setup
    std::uint64_t hash_accum = 14695981039346656037ULL;
    std::uint64_t prime_multiplier = 1099511628211ULL;
    
    for (char character : key) {
        hash_accum ^= static_cast<unsigned char>(character);
        hash_accum *= prime_multiplier;
    }
    
    return static_cast<std::size_t>(hash_accum);
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (!tool) {
        return false;
    }

    std::string plugin_name = tool->name();
    
    // Check if it already exists so we don't insert duplicates
    if (find(plugin_name) != nullptr) {
        return false; 
    }

    // Resize if load factor exceeds 0.75 to keep chains short
    double projected_load = static_cast<double>(size_ + 1) / static_cast<double>(buckets_.size());
    if (projected_load > 0.75) {
        rebuild_buckets(buckets_.size() * 2);
    }

    std::size_t slot_index = hash(plugin_name) % buckets_.size();
    buckets_[slot_index].push_back(std::move(tool));
    
    size_++;
    return true;
}

Tool* ToolRegistry::find(std::string_view name) const {
    if (buckets_.empty()) {
        return nullptr;
    }

    std::size_t slot_index = hash(name) % buckets_.size();
    const auto& chain_list = buckets_[slot_index];
    
    for (const auto& entry_ptr : chain_list) {
        if (entry_ptr && entry_ptr->name() == name) {
            return entry_ptr.get();
        }
    }

    return nullptr;
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const auto& bucket : buckets_) {
        for (const auto& entry_ptr : bucket) {
            if (entry_ptr) {
                fn(*entry_ptr);
            }
        }
    }
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();
}

double ToolRegistry::load_factor() const noexcept {
    if (buckets_.empty()) {
        return 0.0;
    }
    return static_cast<double>(size_) / static_cast<double>(buckets_.size());
}

std::size_t ToolRegistry::max_probe_length() const {
    std::size_t max_chain = 0;
    for (const auto& bucket : buckets_) {
        if (bucket.size() > max_chain) {
            max_chain = bucket.size();
        }
    }
    return max_chain;
}

void ToolRegistry::rebuild_buckets(std::size_t new_bucket_count) {
    std::vector<std::vector<std::unique_ptr<Tool>>> fresh_matrix(new_bucket_count);

    for (auto& old_bucket : buckets_) {
        while (!old_bucket.empty()) {
            std::unique_ptr<Tool> moving_tool = std::move(old_bucket.back());
            old_bucket.pop_back();

            if (moving_tool) {
                std::size_t new_slot = hash(moving_tool->name()) % new_bucket_count;
                fresh_matrix[new_slot].push_back(std::move(moving_tool));
            }
        }
    }

    buckets_ = std::move(fresh_matrix);
}