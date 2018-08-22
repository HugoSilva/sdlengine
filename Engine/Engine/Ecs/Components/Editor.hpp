#pragma once

class EditorElement
{
public:
	EditorElement() { }
	virtual void render() const = 0;
};

class EditorComponent
{
public:
	EditorComponent() { }
	EditorComponent(EditorElement* EditorElement) : m_renderElement(EditorElement) { }
	void render()
	{
		m_renderElement->render();
	}
private:
	EditorElement* m_renderElement{ nullptr };
};

template<typename Archive>
void serialize(Archive &archive, EditorComponent &editor) {
	//archive(editor.m_renderElement);
}