#include "InteractPromptWidget.h"
#include "Components/TextBlock.h"

void UInteractPromptWidget::SetPrompt(const FText& Text)
{
	if (PromptText)
	{
		PromptText->SetText(Text);
	}
}