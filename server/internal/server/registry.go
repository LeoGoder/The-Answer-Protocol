package server

import "sync"

type Registry struct {
	mutex sync.Mutex
	users map[string]struct{}
}

func NewRegistry() *Registry {
	return &Registry{
		users: make(map[string]struct{}),
	}
}

func (r *Registry) Add(username string) bool {
	r.mutex.Lock()
	defer r.mutex.Unlock()

	if _, exists := r.users[username]; exists {
		return false
	}

	r.users[username] = struct{}{}
	return true
}

func (r *Registry) Remove(username string) {
	r.mutex.Lock()
	defer r.mutex.Unlock()

	delete(r.users, username)
}

func (r *Registry) Count() int {
	r.mutex.Lock()
	defer r.mutex.Unlock()

	return len(r.users)
}